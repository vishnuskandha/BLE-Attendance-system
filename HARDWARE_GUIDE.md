# Hardware Setup Guide - BLE Attendance System

## 🛠️ Complete Hardware List

### Main Components

| Component | Specification | Quantity | Estimated Price | Purpose |
|-----------|--------------|----------|-----------------|---------|
| **ESP32 DevKit V1** | 38-pin, WiFi + BLE | 1 | $5-10 | Main controller |
| **DS3231 RTC Module** | I2C, with battery | 1 | $2-5 | Real-time clock |
| **BLE Beacons** | iBeacon/Eddystone | 2+ | $3-8 each | Student ID tags |
| **Micro USB Cable** | Data + Power | 1 | $2-3 | Programming & power |
| **Jumper Wires** | Female-to-Female | 4+ | $1-2 | Connections |

### Optional Components

| Component | Purpose | Price |
|-----------|---------|-------|
| **5V Power Adapter** | Standalone power | $3-5 |
| **Project Box** | Enclosure | $2-5 |
| **LED Indicator** | Status display | $0.50 |
| **Reset Button** | Manual reset | $0.50 |
| **Breadboard** | Prototyping | $2-3 |

**Total Cost**: ~$20-40 for complete system

---

## 📦 ESP32 DevKit V1 Pinout

```
                          ┌─────────────┐
                          │   ESP32     │
                          │  DevKit V1  │
    EN  ─────────────────┤1          38├───────────────── GPIO23
    VP  ─────────────────┤2          37├───────────────── GPIO22 (SCL)
    VN  ─────────────────┤3          36├───────────────── TXD0
   GPIO34 ───────────────┤4          35├───────────────── RXD0
   GPIO35 ───────────────┤5          34├───────────────── GPIO21 (SDA)
   GPIO32 ───────────────┤6          33├───────────────── GND
   GPIO33 ───────────────┤7          32├───────────────── GPIO19
   GPIO25 ───────────────┤8          31├───────────────── GPIO18
   GPIO26 ───────────────┤9          30├───────────────── GPIO5
   GPIO27 ───────────────┤10         29├───────────────── GPIO17
   GPIO14 ───────────────┤11         28├───────────────── GPIO16
   GPIO12 ───────────────┤12         27├───────────────── GPIO4
    GND  ────────────────┤13         26├───────────────── GPIO0
   GPIO13 ───────────────┤14         25├───────────────── GPIO2 (LED)
     D2  ────────────────┤15         24├───────────────── GPIO15
     D3  ────────────────┤16         23├───────────────── D1
    CMD  ────────────────┤17         22├───────────────── D0
    5V   ────────────────┤18         21├───────────────── CLK
                          │19  [USB]  20│
                          └─────────────┘
                            GND      3V3
```

### Key Pins for This Project

| Pin | Function | Connected To |
|-----|----------|--------------|
| **GPIO 21** | I2C SDA | DS3231 SDA |
| **GPIO 22** | I2C SCL | DS3231 SCL |
| **3V3** | 3.3V Power | DS3231 VCC |
| **GND** | Ground | DS3231 GND |
| **GPIO 2** | Built-in LED | Status indicator (optional) |

---

## 🔌 DS3231 RTC Module

### Module Specifications
- **Interface**: I2C
- **Voltage**: 3.3V - 5V
- **Accuracy**: ±2ppm (very accurate)
- **Battery**: CR2032 (included)
- **Temperature**: -40°C to +85°C

### Pin Layout
```
┌─────────────────┐
│   DS3231 RTC    │
│                 │
│  [CR2032]       │
│                 │
├─────────────────┤
│ VCC  GND  SDA   │
│              SCL│
└──┬───┬────┬───┬─┘
   │   │    │   │
   3V3 GND  21  22  (ESP32 Pins)
```

### Wiring Table

| DS3231 Pin | Wire Color (Suggested) | ESP32 Pin |
|------------|----------------------|-----------|
| VCC | Red | 3V3 |
| GND | Black | GND |
| SDA | Yellow/Blue | GPIO 21 |
| SCL | Green/White | GPIO 22 |

---

## 📡 BLE Beacon Setup

### Beacon Specifications

#### Student 1: Mathumitha R
- **MAC Address**: `0E:A5:25:A0:00:16`
- **Roll Number**: 310622205081
- **Attachment**: Student ID card

#### Student 2: Lipsa Sahoo
- **MAC Address**: `0E:A5:25:A0:00:13`
- **Roll Number**: 310622205075
- **Attachment**: Student ID card

### Beacon Configuration

| Setting | Recommended Value | Notes |
|---------|------------------|-------|
| **Advertising Interval** | 100-300ms | Balance between battery & detection speed |
| **TX Power** | -12 to -4 dBm | Adjust for room size |
| **Mode** | iBeacon or Eddystone | Either works |
| **Battery** | CR2032 | Replace every 6-12 months |

### Beacon Placement
```
┌─────────────────────┐
│   Student ID Card   │
│                     │
│   Photo             │
│   Name              │
│   Roll No.          │
│                     │
│   [BLE Beacon]  ←── Attach here (back side)
└─────────────────────┘
```

**Mounting Options**:
1. **Double-sided tape** - Simple, removable
2. **Adhesive sticker** - Permanent
3. **Card pocket** - Replaceable

---

## 🔧 Assembly Instructions

### Step 1: Prepare Components
- [ ] ESP32 DevKit V1
- [ ] DS3231 RTC Module
- [ ] 4x Jumper Wires (Female-to-Female)
- [ ] Micro USB Cable

### Step 2: Connect RTC to ESP32

1. **Power Connections**:
   ```
   DS3231 VCC → ESP32 3V3 (Red wire)
   DS3231 GND → ESP32 GND (Black wire)
   ```

2. **I2C Connections**:
   ```
   DS3231 SDA → ESP32 GPIO21 (Yellow wire)
   DS3231 SCL → ESP32 GPIO22 (Green wire)
   ```

3. **Insert CR2032 battery** into DS3231 module

### Step 3: Verify Connections

**Double-check wiring**:
```
ESP32          DS3231
-----          ------
3V3    ────→   VCC
GND    ────→   GND
GPIO21 ────→   SDA
GPIO22 ────→   SCL
```

### Step 4: Program ESP32

1. Connect ESP32 to computer via USB
2. Open Arduino IDE
3. Load `esp32_ble_scanner.ino`
4. Configure WiFi credentials
5. Upload code
6. Open Serial Monitor (115200 baud)

### Step 5: Test System

1. Power on ESP32
2. Check Serial Monitor for:
   - WiFi connection
   - RTC initialization
   - Web server IP address
3. Access web interface
4. Test beacon detection

---

## 📐 Enclosure Design (Optional)

### Suggested Dimensions
- **Box Size**: 100mm × 60mm × 30mm
- **Material**: ABS plastic or acrylic

### Layout
```
┌──────────────────────────────────┐
│                                  │
│  ┌────────────┐    ┌──────┐     │
│  │   ESP32    │    │ RTC  │     │
│  │  DevKit V1 │    │DS3231│     │
│  └────────────┘    └──────┘     │
│                                  │
│  [USB Port Cutout]               │
└──────────────────────────────────┘
```

### Features
- **Ventilation holes** for heat dissipation
- **USB port access** for power/programming
- **LED window** for status indicator
- **Wall mount** holes (optional)

---

## 🔋 Power Options

### Option 1: USB Power (Recommended)
- **Source**: Micro USB cable
- **Voltage**: 5V DC
- **Current**: 500mA minimum
- **Pros**: Simple, reliable
- **Cons**: Requires nearby USB port

### Option 2: Wall Adapter
- **Source**: 5V 1A USB wall adapter
- **Pros**: Independent power
- **Cons**: Additional cost

### Option 3: Power Bank
- **Source**: USB power bank
- **Pros**: Portable, backup during outages
- **Cons**: Needs periodic recharging

### Option 4: PoE (Power over Ethernet)
- **Source**: PoE splitter to USB
- **Pros**: Clean installation
- **Cons**: Requires PoE network

---

## 🧪 Testing & Calibration

### Initial Tests

#### 1. RTC Test
```cpp
// Test code to verify RTC
Wire.begin(21, 22);
if (rtc.begin()) {
  Serial.println("✓ RTC OK");
  DateTime now = rtc.now();
  Serial.printf("Time: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
} else {
  Serial.println("✗ RTC FAIL");
}
```

#### 2. WiFi Test
```cpp
WiFi.begin(SSID, PASSWORD);
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}
Serial.println("\n✓ WiFi Connected");
Serial.print("IP: ");
Serial.println(WiFi.localIP());
```

#### 3. BLE Scan Test
```cpp
BLEScanResults results = pBLEScan->start(5, false);
Serial.printf("Found %d devices\n", results.getCount());
for (int i = 0; i < results.getCount(); i++) {
  BLEAdvertisedDevice device = results.getDevice(i);
  Serial.printf("%s - RSSI: %d\n", device.getAddress().toString().c_str(), device.getRSSI());
}
```

### Calibration

#### RSSI Threshold Adjustment

Test at different distances:

| Distance | Typical RSSI | Threshold Setting |
|----------|-------------|-------------------|
| 0-2m (very close) | -40 to -60 | -70 |
| 2-5m (near) | -60 to -75 | -80 |
| 5-10m (medium) | -75 to -85 | -85 |
| 10-15m (far) | -85 to -95 | -90 |

**Test procedure**:
1. Place beacon at desired maximum distance
2. Note RSSI value in Serial Monitor
3. Set threshold 5-10 dBm lower
4. Test false positives/negatives

### ESP32 Memory Notes

| Resource | Usage |
|---|---|
| BLE subsystem | ~130KB RAM |
| WiFi + TCP | ~40KB RAM |
| SSL/TLS buffers | ~50KB needed |
| Free (BLE active) | ~45KB |
| Free (BLE off) | ~175KB |

The firmware (`esp32_attendance_optimized.ino`) time-shares BLE and SSL: it deinitializes BLE before HTTPS sends, then reinitializes it for the next scan.

---

## 🛡️ Troubleshooting Hardware

### Problem: RTC Not Detected

**Symptoms**:
- Serial shows "RTC not found!"
- Time shows placeholder values

**Solutions**:
1. Check wire connections (SDA/SCL)
2. Verify 3.3V power on VCC pin
3. Try swapping SDA/SCL wires
4. Test with I2C scanner code
5. Check for loose wires

### Problem: Beacons Not Detected

**Symptoms**:
- BLE scan finds 0 devices
- Specific beacon missing

**Solutions**:
1. Check beacon battery
2. Verify MAC address matches
3. Increase RSSI threshold to -95
4. Move beacon closer (< 1 meter)
5. Check beacon is advertising
6. Use nRF Connect app to verify beacon

### Problem: ESP32 Won't Program

**Symptoms**:
- Upload fails
- "Failed to connect" error

**Solutions**:
1. Hold BOOT button during upload
2. Check USB cable (must be data cable)
3. Install/update USB drivers
4. Try different USB port
5. Lower upload speed to 115200

### Problem: WiFi Disconnects

**Symptoms**:
- Periodic WiFi drops
- Web server inaccessible

**Solutions**:
1. Move closer to router
2. Use 2.4GHz WiFi (not 5GHz)
3. Set static IP
4. Add WiFi reconnect code
5. Check router settings

---

## 📏 Installation Guidelines

### Classroom Installation

#### Optimal ESP32 Placement
```
┌────────────────────────────────────┐
│         Classroom Layout           │
│                                    │
│  Teacher     [ESP32]               │
│   Desk    ← Mount here             │
│                                    │
│  ┌──┐  ┌──┐  ┌──┐  ┌──┐          │
│  │  │  │  │  │  │  │  │ Student   │
│  └──┘  └──┘  └──┘  └──┘ Desks    │
│  ┌──┐  ┌──┐  ┌──┐  ┌──┐          │
│  │  │  │  │  │  │  │  │           │
│  └──┘  └──┘  └──┘  └──┘          │
│                                    │
└────────────────────────────────────┘

Detection Range: 5-10 meters
```

#### Mounting Options
1. **Wall mount**: Near teacher's desk, 1.5m height
2. **Desk mount**: On teacher's desk
3. **Ceiling mount**: Central location (requires long USB cable)

#### Considerations
- **Height**: 1-2 meters for optimal signal
- **Obstacles**: Avoid metal partitions
- **Power**: Near power outlet
- **Network**: WiFi coverage area

---

## 🔐 Security & Maintenance

### Physical Security
- Mount in secure location
- Use enclosure box
- Secure USB cable
- Restrict physical access

### Maintenance Schedule

| Task | Frequency | Notes |
|------|-----------|-------|
| Check RTC battery | Every 6 months | CR2032 |
| Test beacon batteries | Monthly | Replace when RSSI drops |
| Clean components | Every 3 months | Dust removal |
| Backup settings | After changes | Keep config file |
| Update firmware | As needed | Bug fixes/features |

---

## 📚 Additional Resources

### Datasheets
- **ESP32**: https://www.espressif.com/en/products/socs/esp32
- **DS3231**: https://datasheets.maximintegrated.com/en/ds/DS3231.pdf

### Tools
- **nRF Connect** (Mobile app): Scan and configure BLE beacons
- **I2C Scanner**: Detect I2C device addresses
- **Serial Monitor**: Arduino IDE built-in tool

---

**Last Updated**: February 13, 2026  
**Hardware Version**: 1.0
