# ESP32 BLE Attendance System - Setup Guide

## 📋 Table of Contents
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Software Installation](#software-installation)
- [Configuration](#configuration)
- [Features Overview](#features-overview)
- [How It Works](#how-it-works)

---

## 🔧 Hardware Requirements

### Essential Components
1. **ESP32 DevKit V1** - Main controller
2. **DS3231 RTC Module** - Real-time clock for accurate timestamps
3. **BLE Beacons** (2x) - Attached to student ID cards
   - Beacon 1: `0E:A5:25:A0:00:16` (Mathumitha R)
   - Beacon 2: `0E:A5:25:A0:00:13` (Lipsa Sahoo)
4. **Micro USB Cable** - For power and programming
5. **Jumper Wires** - For RTC connections

### Optional Components
- **External 5V Power Supply** - For standalone operation
- **Enclosure Box** - To protect the circuit

---

## 🔌 Wiring Diagram

### DS3231 RTC Module Connection

```
DS3231 RTC          ESP32 DevKit V1
-----------         ---------------
VCC (3.3V)    →     3V3
GND           →     GND
SDA           →     GPIO 21 (SDA)
SCL           →     GPIO 22 (SCL)
```

### Pin Configuration
```
ESP32 Pin    Function
---------    --------
GPIO 21      I2C SDA (Data)
GPIO 22      I2C SCL (Clock)
3V3          Power for RTC
GND          Ground
```

### Connection Diagram
```
┌─────────────────┐
│   ESP32 DevKit  │
│      V1         │
│                 │
│  21 (SDA) ──────┼────── SDA
│  22 (SCL) ──────┼────── SCL     DS3231
│  3V3      ──────┼────── VCC     RTC Module
│  GND      ──────┼────── GND
└─────────────────┘
```

---

## 💻 Software Installation

### 1. Install Arduino IDE
Download from: https://www.arduino.cc/en/software

### 2. Install ESP32 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search "ESP32" and install "ESP32 by Espressif Systems"

### 3. Install Required Libraries
Go to **Sketch → Include Library → Manage Libraries** and install:

| Library | Version | Purpose |
|---------|---------|---------|
| **RTClib** | by Adafruit | DS3231 RTC interface |
| **ArduinoJson** | by Benoit Blanchon | JSON parsing |
| **WebServer** | Built-in with ESP32 | Local web server |

### 4. Configure Board Settings
- **Board**: ESP32 Dev Module
- **Upload Speed**: 921600
- **CPU Frequency**: 240MHz
- **Flash Frequency**: 80MHz
- **Flash Mode**: QIO
- **Flash Size**: 4MB
- **Port**: Select your COM port

---

## ⚙️ Configuration

### Step 1: Update WiFi Credentials
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";      // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // Your WiFi password
```

### Step 2: Set Server URL
```cpp
const char* SERVER_URL = "https://ble-attendance-system-pink.vercel.app/api/attendance";
```
Replace with your Vercel deployment URL. The ESP32 uses `WiFiClientSecure` with `setInsecure()` for HTTPS.

> **⚠️ Important**: The network you connect to must allow outbound HTTPS connections. Some mobile hotspots and institutional networks block this. Test with `curl` from a laptop on the same network first.

### Step 3: Configure Location
```cpp
const char* LOCATION_ID = "CLASSROOM_A";  // Room identifier
const char* DEPARTMENT  = "IT-B";         // Department code
```

### Step 4: Adjust Scan Settings (Optional)
```cpp
const int SCAN_DURATION  = 5;      // Scan duration in seconds
const int SCAN_INTERVAL  = 60000;  // 1 minute between scans
const int RSSI_THRESHOLD = -80;    // Signal strength (-100 to -30 dBm)
```

**RSSI Guidelines:**
- `-50 dBm` = Very close (< 1 meter)
- `-70 dBm` = Good signal (3-5 meters)
- `-80 dBm` = Acceptable (5-10 meters)
- `-90 dBm` = Weak (> 10 meters)

---

## ✨ Features Overview

### 1. **Automatic BLE Scanning** (Every 1 Minute)
- Scans for registered student beacons
- Detects presence based on signal strength
- **Deinitializes BLE** after scanning to free ~130KB RAM
- Sends attendance data via HTTPS
- **Reinitializes BLE** for the next scan cycle

### 2. **RTC Time Tracking**
- Accurate date and time using DS3231
- Battery-backed (keeps time during power loss)
- Period detection (Period 1, 2, 3)

### 3. **Local Web Interface** 
Access at: `http://ESP32_IP_ADDRESS`

Staff can:
- Grant on-duty permissions
- Set custom duration (minutes)
- Choose period/half-day/full-day
- View current student status

### 4. **Smart Data Retention (14 Days)**
The backend automatically maintains a **rolling 14-day window**:
- Data older than 2 weeks is automatically removed.
- Ensures the system stays fast and stays within free-tier limits.
- The dashboard "Date Range" tool allows you to view any segment of this 2-week history.

### 5. **Smart Attendance Logic**
```
Student Present IF:
  ✓ Beacon detected in range, OR
  ✓ Valid on-duty permission active

Student Absent IF:
  ✗ Beacon NOT detected AND
  ✗ No active permission
```

### 5. **Status Code System**
Each student has a unique ID code:
- **1P** = Student 1 Present
- **1A** = Student 1 Absent
- **2P** = Student 2 Present
- **2A** = Student 2 Absent

---

## 🔄 How It Works

### System Flow
```
 ┌────────────────────────────────────────────────────────────┐
 │                    EVERY 1 MINUTE                          │
 └────────────────────────────────────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────┐
        │   1. BLE Scan (5 seconds)         │
        │   - Detect beacons in range       │
        │   - Check signal strength (RSSI)  │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   2. Collect Results              │
        │   - Store present/absent in array │
        │   - Check on-duty permissions     │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   3. BLEDevice::deinit(true)      │
        │   - Shut down BLE completely      │
        │   - Free ~130KB RAM for SSL       │
        │   - Heap: 45KB → 175KB            │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   4. HTTPS POST (WiFiClientSecure)│
        │   - SSL needs ~50KB heap          │
        │   - Send each student's data      │
        │   - POST OK (200) confirms        │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   5. initBLE() — Restart BLE      │
        │   - Reinitialize for next scan    │
        │   - Heap drops back to ~45KB      │
        └───────────────────────────────────┘
```

### Permission System Flow
```
┌─────────────────────────────────────────────────────────┐
│  Teacher opens: http://192.168.1.X                      │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│  Web Interface:                                         │
│  - Select Student                                        │
│  - Choose Permission Type:                               │
│    • Single Period (120 min)                            │
│    • Half Day (240 min)                                 │
│    • Full Day (480 min)                                 │
│    • Custom Duration                                    │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│  ESP32 Records:                                         │
│  - Student ID                                           │
│  - onDuty = true                                        │
│  - onDutyUntil = current_time + duration               │
└────────────────┬────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────┐
│  During Each Scan:                                      │
│  IF current_time < onDutyUntil:                        │
│     Mark student as PRESENT (even without beacon)       │
│  ELSE:                                                  │
│     Clear on-duty flag                                  │
└─────────────────────────────────────────────────────────┘
```

---

## 📊 Period Timings

| Period | Start Time | End Time | Duration |
|--------|-----------|----------|----------|
| Period 1 | 8:15 AM | 10:15 AM | 2 hours |
| Period 2 | 10:30 AM | 12:45 PM | 2h 15m |
| Period 3 | 1:30 PM | 3:45 PM | 2h 15m |

---

## 🚀 First Time Setup

1. **Wire the RTC module**
2. **Load `esp32_attendance_optimized.ino`**
3. Configure WiFi credentials
4. Upload code
5. Open Serial Monitor (115200 baud)
6. **Note the IP address** displayed
7. **Access web interface**: `http://[ESP32_IP]`
8. **Test beacon detection** by bringing ID cards near
9. **Grant test permission** via web interface

---

## 🐛 Troubleshooting

### RTC Not Found
- Check wiring connections
- Ensure RTC module has power (LED on)
- Try swapping SDA/SCL if not detected

### Problem: HTTPS POST Fails (connection refused)

**Symptoms**:
- Serial shows `POST FAIL (connection refused)`
- NTP time sync works but HTTPS doesn't

**Solutions**:
1. Check free heap in Serial Monitor — needs >50KB when BLE is off
2. Ensure `BLEDevice::deinit(true)` is called before HTTPS
3. Verify network allows outbound HTTPS (test with curl from laptop)
4. Try a different WiFi network (some hotspots block HTTPS)
5. The ESP32 Core v3.x needs `WiFiClientSecure` with `setInsecure()`

### Problem: WiFi Disconnecting
- Verify SSID and password
- Check 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Move closer to router

### Beacons Not Detected
- Check beacon batteries
- Verify MAC addresses match
- Increase RSSI threshold to -90 or -95
- Ensure beacons are advertising

### Web Interface Not Loading
- Check ESP32 IP address in Serial Monitor
- Ensure device is on same network
- Try `http://192.168.1.X` format

---

## 📝 Notes

- **Battery Backup**: DS3231 has built-in battery for timekeeping
- **Auto-Recovery**: System restarts automatically on crash
- **Scan Frequency**: Adjustable (default 1 minute)
- **Power Requirements**: 5V/500mA via USB or external supply

---

**Last Updated**: February 24, 2026
