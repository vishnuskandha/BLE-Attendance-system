# ESP32 BLE Attendance System — Code Explanation

**File:** `esp32_attendance_optimized.ino`
**Version:** 3.0 (Unified Redis Enterprise + 14-Day Retention)
**Date:** 24-Feb-2026

---

## Table of Contents
1. [System Overview](#1-system-overview)
2. [Includes & Libraries](#2-includes--libraries)
3. [Configuration Constants](#3-configuration-constants)
4. [Data Structures](#4-data-structures)
5. [Global Variables](#5-global-variables)
6. [HTML Template](#6-html-template)
7. [RTC Setup](#7-rtc-setup)
8. [WiFi Connection](#8-wifi-connection)
9. [Time Helper Functions](#9-time-helper-functions)
10. [Attendance Sending (HTTPS)](#10-attendance-sending-https)
11. [Web Server Handlers](#11-web-server-handlers)
12. [BLE Callback & Init](#12-ble-callback--init)
13. [Setup Function](#13-setup-function)
14. [Main Loop — 5-Step Scan Cycle](#14-main-loop--5-step-scan-cycle)
15. [Memory Management Strategy](#15-memory-management-strategy)

---

## 1. System Overview

This ESP32-based attendance system automates student attendance tracking using BLE (Bluetooth Low Energy) beacons. Each student carries a BLE beacon, and the ESP32 scans for these beacons every 1 minute to determine who is present.

### Architecture
```
┌─────────────┐     BLE Scan     ┌──────────────┐     HTTPS POST     ┌─────────────┐
│ BLE Beacons │ ──────────────── │    ESP32      │ ──────────────────│ Vercel API  │
│ (Students)  │                  │  Controller   │                   │  (Cloud)    │
└─────────────┘                  └──────────────┘                   └─────────────┘
                                       │
                                       │ WiFi (HTTP)
                                       ▼
                                 ┌──────────────┐
                                 │  Web Portal   │
                                 │ (Staff Phone) │
                                 └──────────────┘
```

### Key Features
- **BLE Scanning:** Detects student beacons within RSSI range every 60 seconds
- **HTTPS Posting:** Sends attendance data to a Vercel serverless API
- **RTC Timekeeping:** DS3231 module with battery backup for accurate time
- **Web Portal:** Staff can grant on-duty permissions via phone browser
- **Memory Management:** BLE deinit/reinit cycle to allow SSL connections
- **Data Retention:** Backend automatically prunes records older than 14 days

---

## 2. Includes & Libraries

```cpp
#include <ArduinoJson.h>       // JSON serialization for API payloads
#include <BLEDevice.h>         // BLE initialization and device management
#include <BLEScan.h>           // BLE scanning functionality
#include <BLEUtils.h>          // BLE utility functions
#include <HTTPClient.h>        // HTTP/HTTPS client for API calls
#include <Preferences.h>       // Non-volatile storage (NVS) for RTC state
#include <RTClib.h>            // DS3231 RTC module driver
#include <WebServer.h>         // ESP32 local web server
#include <WiFi.h>              // WiFi connectivity
#include <WiFiClientSecure.h>  // SSL/TLS support for HTTPS
#include <Wire.h>              // I2C communication (for RTC)
```

**Why `WiFiClientSecure`?** The Vercel API uses HTTPS. The ESP32 needs this library to establish SSL/TLS connections. We use `client.setInsecure()` to skip certificate validation since embedded devices have limited certificate storage.

---

## 3. Configuration Constants

```cpp
const char *WIFI_SSID = "ACT_003F";       // WiFi network name
const char *WIFI_PASSWORD = "kTYUFiT4";   // WiFi password
const char *SERVER_URL = "https://ble-attendance-system-pink.vercel.app/api/attendance";
const char *LOCATION_ID = "CLASSROOM_A";   // Physical location identifier
const char *DEPARTMENT = "IT-B";           // Department code

#define RTC_SDA 21              // I2C data pin for RTC
#define RTC_SCL 22              // I2C clock pin for RTC
const int SCAN_DURATION = 5;    // BLE scan duration in seconds
const int SCAN_INTERVAL = 60000;// Time between scans (1 minute)
const int RSSI_THRESHOLD = -80; // Minimum signal strength to accept
```

| Constant | Purpose |
|---|---|
| `WIFI_SSID/PASSWORD` | WiFi credentials — change for your network |
| `SERVER_URL` | Vercel API endpoint that receives attendance data |
| `LOCATION_ID` | Identifies which classroom this ESP32 is in |
| `SCAN_DURATION` | 5 seconds of BLE scanning per cycle |
| `SCAN_INTERVAL` | 60,000ms = 1 minute between scans |
| `RSSI_THRESHOLD` | -80 dBm — beacons weaker than this are ignored |

---

## 4. Data Structures

### StudentBeacon
```cpp
struct StudentBeacon {
  int id;                     // Unique student ID (1, 2, 3...)
  String mac;                 // BLE beacon MAC address
  String name;                // Student's full name
  String roll;                // Roll number
  int year;                   // Academic year
  bool onDuty;                // Currently on-duty/permission
  unsigned long onDutyUntil;  // Unix timestamp when permission expires
};
```

Each student has a BLE beacon with a unique MAC address. The `registry[]` array maps MAC addresses to student information.

### Period
```cpp
struct Period {
  int startHour, startMin, endHour, endMin;
  const char *name;
};
```

Defines class periods (Period 1: 8:15–10:15, Period 2: 10:30–12:45, Period 3: 13:30–15:45). The system uses these to tag attendance records with the current period.

---

## 5. Global Variables

```cpp
BLEScan *pBLEScan;        // BLE scanner object — recreated each cycle
RTC_DS3231 rtc;           // RTC hardware interface
WebServer server(80);      // Local web server on port 80
Preferences prefs;         // NVS storage for persistent settings
bool wifiReady = false;    // WiFi connection status
bool rtcReady = false;     // RTC hardware detected
bool rtcTimeSet = false;   // RTC has valid time set
String detected[64];       // MAC addresses detected this scan
int detectedCount = 0;     // Number of beacons found this scan
```

**Important:** `pBLEScan` is a pointer because the BLE subsystem is completely destroyed and recreated each scan cycle (see [Memory Management](#15-memory-management-strategy)).

---

## 6. HTML Template

```cpp
const char HTML_HEADER[] PROGMEM = "<!DOCTYPE html>...";
```

The HTML template is stored in `PROGMEM` (flash memory, not RAM) to save heap space. It contains:
- A purple gradient background with a white card layout
- Responsive CSS for mobile phones
- Warning styles (`.w`) for alerts like "RTC not set"
- On-duty status indicators (`.on` green, `.off` gray)

---

## 7. RTC Setup

```cpp
void setupRTC() {
  Wire.begin(RTC_SDA, RTC_SCL);   // Start I2C on pins 21, 22
  if (!rtc.begin()) { ... }        // Check if RTC is connected

  // Check if RTC lost power (battery dead/missing)
  if (rtc.lostPower() || rtc.now().year() < 2020) {
    rtcTimeSet = false;            // Require manual time set via web portal
  } else {
    rtcTimeSet = true;             // Time is valid
  }

  // Persist RTC state to NVS
  prefs.begin("rtc", false);
  prefs.putBool("set", true);
  prefs.end();
}
```

**Why check `year() < 2020`?** If the RTC returns a year before 2020, the backup battery is dead and the time has reset to a factory default. The system blocks scanning until time is set via the web portal.

---

## 8. WiFi Connection

```cpp
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Wait up to 15 seconds for connection
  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
  }

  // Test internet connectivity via NTP
  configTime(0, 0, "pool.ntp.org");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time - Internet Check Failed?");
  } else {
    Serial.println("Time synced - Internet OK");
  }
}
```

After connecting to WiFi, the code performs an NTP time sync as a **connectivity diagnostic** — if NTP succeeds, we know the ESP32 has real internet access (not just a LAN connection).

---

## 9. Time Helper Functions

### `getDateStr()` — Returns date as `DD-MM-YYYY`
### `getTimeStr()` — Returns time as `HH:MM AM/PM`
### `getCurrentPeriod()` — Returns 1, 2, or 3 based on current time

```cpp
int getCurrentPeriod() {
  int currentMin = now.hour() * 60 + now.minute(); // Convert to minutes since midnight
  for (int i = 0; i < PERIOD_COUNT; i++) {
    if (currentMin >= startMin && currentMin < endMin)
      return i + 1;   // Returns 1, 2, or 3
  }
  return 0; // Outside class hours
}
```

### `isOnDutyActive()` — Checks if a student's on-duty permission is still valid

Compares current Unix timestamp against the `onDutyUntil` deadline. Automatically clears expired permissions.

---

## 10. Attendance Sending (HTTPS)

```cpp
void sendAttendance(const StudentBeacon &s, bool present, bool onDuty) {
  if (!rtcTimeSet || WiFi.status() != WL_CONNECTED) return;

  // Build JSON payload
  StaticJsonDocument<512> doc;
  doc["studentId"] = s.id;
  doc["code"] = String(s.id) + (present ? "P" : "A");  // e.g., "1P" or "1A"
  doc["name"] = s.name;
  // ... other fields ...

  // HTTPS connection with SSL
  WiFiClientSecure client;
  client.setInsecure();       // Skip certificate validation
  client.setTimeout(15);      // 15 second SSL timeout

  HTTPClient http;
  http.setTimeout(15000);     // 15 second HTTP timeout
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);
  // Log result
  http.end();
}
```

**Retention Handling:** Note that the ESP32 doesn't need to worry about database size. The backend API handles the "14-day rolling window" automatically whenever it receives a POST.

**Critical:** This function is called **after** BLE has been shut down (see the main loop). This is because SSL requires ~50KB+ of free RAM, and BLE uses ~130KB. Both cannot run simultaneously.

### JSON Payload Example
```json
{
  "studentId": 1,
  "code": "1P",
  "name": "Mathumitha R",
  "rollNumber": "310622205081",
  "year": 4,
  "department": "IT-B",
  "location": "CLASSROOM_A",
  "status": "Present",
  "onDuty": false,
  "period": 3,
  "date": "13-02-2026",
  "time": "02:43 PM"
}
```

---

## 11. Web Server Handlers

### `handleRoot()` — Main portal page (`/`)
Displays:
- Warning if RTC time is not set
- Current date and time
- Student selection dropdown
- Permission type selector (Single Period / Half Day / Full Day)
- Current on-duty status for all students

### `handleSetTimePage()` — Time configuration page (`/settime`)
Shows a form with date and time inputs pre-filled with current RTC values.

### `handleSaveTime()` — Saves time to RTC (`/savetime`)
Parses the form input (YYYY-MM-DD and HH:MM), sets the RTC, and persists the state to NVS.

### `handleGrant()` — Grants on-duty permission (`/grant`)
Sets `onDuty = true` and calculates the expiry timestamp:
```cpp
registry[i].onDutyUntil = now.unixtime() + (duration * 60);
```
When a student is on-duty, they are marked **Present** even if their beacon is not detected.

---

## 12. BLE Callback & Init

### AdvCallbacks (BLE Scan Callback)
```cpp
class AdvCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    String mac = dev.getAddress().toString().c_str();
    mac.toUpperCase();
    int rssi = dev.getRSSI();

    if (rssi < RSSI_THRESHOLD) return;  // Too weak, ignore

    // Check if MAC matches any registered student
    for (int i = 0; i < REG_COUNT; i++) {
      if (mac == registry[i].mac) {
        // Deduplicate — only count first detection per scan
        if (!seen && detectedCount < 64) {
          detected[detectedCount++] = mac;
        }
        break;
      }
    }
  }
};
```

This callback fires for **every** BLE advertisement detected. It filters by:
1. **RSSI threshold** — ignores beacons too far away
2. **Registry match** — only tracks known students
3. **Deduplication** — counts each beacon only once per scan cycle

### initBLE() — (Re)initializes the BLE subsystem
```cpp
void initBLE() {
  BLEDevice::init("ESP32_Attendance");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(80);
}
```

This function is called during `setup()` and also after every HTTPS send cycle to **restart BLE**. It's separated into its own function because it's called from two places.

---

## 13. Setup Function

```cpp
void setup() {
  Serial.begin(115200);

  setupRTC();        // 1. Initialize RTC
  connectWiFi();     // 2. Connect to WiFi

  // 3. Start web server with route handlers
  server.on("/", handleRoot);
  server.on("/grant", HTTP_POST, handleGrant);
  server.on("/settime", handleSetTimePage);
  server.on("/savetime", HTTP_POST, handleSaveTime);
  server.begin();

  initBLE();         // 4. Initialize BLE scanner

  // 5. Warn if time needs to be set
  if (!rtcTimeSet) {
    Serial.println("*** SET TIME FIRST ***");
  }
}
```

---

## 14. Main Loop — 5-Step Scan Cycle

This is the heart of the system. Every 60 seconds, it runs a 5-step process:

```
┌──────────────────────────────────────────────────┐
│              SCAN CYCLE (every 60s)              │
├──────────────────────────────────────────────────┤
│ STEP 1: BLE Scan (5 seconds)                     │
│   └─ Detect beacons, record MACs                 │
│                                                  │
│ STEP 2: Collect Results                          │
│   └─ Check each student: beacon OR on-duty?      │
│   └─ Store present[] and duty[] arrays           │
│                                                  │
│ STEP 3: BLEDevice::deinit(true)                  │
│   └─ Shuts down BLE, frees ~130KB RAM            │
│   └─ Heap goes from ~45KB to ~175KB              │
│                                                  │
│ STEP 4: HTTPS POST (for each student)            │
│   └─ SSL handshake needs ~50KB (now available)   │
│   └─ Sends JSON payload to Vercel API            │
│                                                  │
│ STEP 5: initBLE()                                │
│   └─ Restarts BLE for next scan cycle            │
│   └─ Heap drops back to ~45KB                    │
└──────────────────────────────────────────────────┘
```

### Code Walkthrough

```cpp
void loop() {
  server.handleClient();  // Always handle web requests

  static unsigned long lastScan = 0;
  if (millis() - lastScan >= SCAN_INTERVAL) {
    lastScan = millis();

    if (!rtcTimeSet) {
      Serial.println("Scan skipped - set RTC time first");
      return;
    }

    // STEP 1: Scan for BLE beacons
    detectedCount = 0;
    pBLEScan->start(SCAN_DURATION, false);
    pBLEScan->clearResults();

    // STEP 2: Collect results
    bool present[REG_COUNT], duty[REG_COUNT];
    for (int i = 0; i < REG_COUNT; i++) {
      // Check if beacon was found + on-duty status
      duty[i] = isOnDutyActive(registry[i]);
      present[i] = beaconDetected || duty[i];
    }

    // STEP 3: Shut down BLE to free ~130KB RAM for SSL
    BLEDevice::deinit(true);

    // STEP 4: Send attendance over HTTPS
    for (int i = 0; i < REG_COUNT; i++) {
      sendAttendance(registry[i], present[i], duty[i]);
    }

    // STEP 5: Restart BLE for next scan
    initBLE();
  }
  delay(100);
}
```

---

## 15. Memory Management Strategy

### The Problem
The ESP32 has ~320KB of usable SRAM. The breakdown:

| Component | RAM Usage |
|---|---|
| BLE subsystem | ~130KB |
| WiFi + TCP stack | ~40KB |
| WebServer + HTML | ~20KB |
| SSL/TLS buffers | ~50KB needed |
| Application code | ~30KB |
| **Total if all active** | **~270KB** (exceeds safe limits) |

When BLE is running, only **~45KB** is free — not enough for SSL's ~50KB buffer requirement. The HTTPS POST fails silently with "connection refused" because the SSL library cannot allocate its handshake buffers.

### The Solution
Instead of running BLE and SSL simultaneously, we **time-share**:

```
BLE ON ──── scan ──── BLE OFF ──── HTTPS ──── BLE ON ──── idle ────
  45KB free           175KB free              45KB free
                      ↑ Enough for SSL!
```

1. **`BLEDevice::deinit(true)`** — Completely shuts down the BLE stack and frees all its memory. The `true` parameter ensures memory is released immediately.
2. **`initBLE()`** — Reinitializes BLE from scratch, re-registering callbacks and scan parameters.

### Memory Measurements (Actual)
```
After BLE scan:     Free heap: 45,232 bytes  ← SSL fails here
After BLE deinit:   Free heap: 175,956 bytes ← SSL works here
After HTTPS send:   Free heap: 174,444 bytes ← Stable
After BLE reinit:   Free heap: 174,444 bytes ← Ready for next cycle
```

This approach adds ~200ms overhead per cycle for BLE teardown/restart, which is negligible compared to the 60-second scan interval.

---

## Summary of Serial Monitor Output

A normal scan cycle produces:
```
========== SCAN ==========
02:43 PM | Period: 3
Beacon: Mathumitha R RSSI:-45
Beacon: Lipsa Sahoo RSSI:-48
Found 2 beacon(s)
[1P] Mathumitha R: PRESENT (BEACON)
[2P] Lipsa Sahoo: PRESENT (BEACON)
BLE off - Free heap: 175956 bytes
  POST OK (200): Mathumitha R
  POST OK (200): Lipsa Sahoo
BLE on - Free heap: 174444 bytes
Next scan in 1 min
```
