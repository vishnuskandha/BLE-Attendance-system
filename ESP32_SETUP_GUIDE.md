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
const char* SERVER_URL = "http://192.168.1.100:3000/api/attendance";
```
Replace `192.168.1.100` with your server's IP address.

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
- Sends attendance data automatically

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

### 4. **Smart Attendance Logic**
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
        │   2. Check Each Student           │
        │   - Beacon detected? YES/NO       │
        │   - On-duty active? YES/NO        │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   3. Determine Status             │
        │   Present = Beacon OR On-duty     │
        │   Absent = Neither condition met  │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │   4. Send to Server (HTTP POST)   │
        │   {                               │
        │     "studentId": 1,               │
        │     "code": "1P",                 │
        │     "status": "Present",          │
        │     "onDuty": false,              │
        │     "period": 2,                  │
        │     "date": "02-01-2026",         │
        │     "time": "10:45 AM"            │
        │   }                               │
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

1. **Wire the RTC module** to ESP32
2. **Upload the code** via Arduino IDE
3. **Open Serial Monitor** (115200 baud)
4. **Note the IP address** displayed
5. **Access web interface**: `http://[ESP32_IP]`
6. **Test beacon detection** by bringing ID cards near
7. **Grant test permission** via web interface

---

## 🐛 Troubleshooting

### RTC Not Found
- Check wiring connections
- Ensure RTC module has power (LED on)
- Try swapping SDA/SCL if not detected

### WiFi Not Connecting
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

**Last Updated**: January 2, 2026
