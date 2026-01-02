/*
 * ESP32 BLE Attendance Scanner with RTC & Web Interface
 * Features:
 * - Scans for student beacons every 1 minute
 * - RTC DS3231 for accurate timekeeping
 * - Local web server for staff permissions/on-duty
 * - Auto-marks present during permission time
 * - Posts attendance data to remote server
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <RTClib.h>
#include <Wire.h>

// ---------- CONFIGURE THESE ----------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL    = "http://YOUR_SERVER_IP:3000/api/attendance"; // POST endpoint
const char* LOCATION_ID   = "CLASSROOM_A";
const char* DEPARTMENT    = "IT-B";

// RTC I2C Pins (ESP32 default)
#define RTC_SDA 21
#define RTC_SCL 22

// Scan behavior
const int SCAN_DURATION   = 5;      // seconds per scan
const int SCAN_INTERVAL   = 60000;  // 1 minute between scans
const int RSSI_THRESHOLD  = -80;    // ignore weaker signals (adjust to your room)

// ---------- STUDENT REGISTRY ----------
struct StudentBeacon {
  int id;           // Student ID (1, 2, 3...)
  String mac;       // Beacon MAC address
  String name;      // Student name
  String roll;      // Roll number
  int year;         // Year
  bool onDuty;      // On-duty flag
  unsigned long onDutyUntil; // Unix timestamp when on-duty ends
};

StudentBeacon registry[] = {
  {1, "0E:A5:25:A0:00:16", "Mathumitha R", "310622205081", 4, false, 0},
  {2, "0E:A5:25:A0:00:13", "Lipsa Sahoo", "310622205075", 4, false, 0}
};
const int REG_COUNT = sizeof(registry) / sizeof(registry[0]);

// Period timings (24-hour format)
struct Period {
  int startHour, startMin;
  int endHour, endMin;
  String name;
};

Period periods[] = {
  {8, 15, 10, 15, "Period 1"},
  {10, 30, 12, 45, "Period 2"},
  {13, 30, 15, 45, "Period 3"}
};
const int PERIOD_COUNT = sizeof(periods) / sizeof(periods[0]);

// ---------- GLOBALS ----------
BLEScan* pBLESRTC SETUP ----------
void setupRTC() {
  Wire.begin(RTC_SDA, RTC_SCL);
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    rtcReady = false;
    return;
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time...");
    // Set to compile time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  rtcReady = true;
  Serial.println("RTC initialized");
}

// ---------- TIME HELPERS ----------
String getDateStr() {
  if (!rtcReady) return "02-01-2026";
  DateTime now = rtc.now();
  char buf[16];
  sprintf(buf, "%02d-%02d-%04d", now.day(), now.month(), now.year());
  return String(buf);
}

String getTimeStr() {
  if (!rtcReady) return "09:00 AM";
  DateTime now = rtc.now();
  int h = now.hour();bool present, bool onDuty = false) {
  if (!wifiReady || WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not ready, skip send");
    return;
  }
  
  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;
  doc["studentId"] = s.id;
  doc["code"] = String(s.id) + (present ? "P" : "A"); // e.g., "1P" or "2A"
  doc["name"] = s.name;
  doc["rollNumber"] = s.roll;
  doc["year"] = s.year;
  doc["department"] = DEPARTMENT;
  doc["location"] = LOCATION_ID;
  doc["status"] = present ? "Present" : "Absent";
  doc["onDuty"] = onDuty;
  doc["period"] = getCurrentPeriod()< PERIOD_COUNT; i++) {
    int startMin = periods[i].startHour * 60 + periods[i].startMin;
    int endMin = periods[i].endHour * 60 + periods[i].endMin;
    if (currentMin >= startMin && currentMin < endMin) {
      return i + 1; // Return period number (1, 2, 3)
    }
  }WEB SERVER HANDLERS ----------
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Attendance Permission Portal</title>
  <style>
    body { font-family: Arial; padding: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 15px; box-shadow: 0 10px 40px rgba(0,0,0,0.3); }
    h1 { color: #667eea; text-align: center; }
    .student { background: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 8px; border-left: 4px solid #667eea; }
    select, input { width: 100%; padding: 10px; margin: 5px 0; border: 2px solid #ddd; border-radius: 5px; }
    button { background: #667eea; color: white; padding: 12px 30px; border: none; border-radius: 8px; cursor: pointer; width: 100%; font-size: 16px; margin-top: 10px; }
    button:hover { background: #764ba2; }
    .info { color: #666; font-size: 14px; margin-top: 5px; }
  </style>
</head>
<body>
  <div class='container'>
    <h1>📋 Attendance Permission Portal</h1>
    <p class='info'>Grant on-duty or permission for students</p>
    
    <form method='POST' action='/grant'>
      <div class='student'>
        <h3>Select Student:</h3>
        <select name='studentId' required>
          <option value=''>-- Choose Student --</option>
)";

  for (int i = 0; i < REG_COUNT; i++) {
    html += "<option value='" + String(registry[i].id) + "'>" + registry[i].name + " (" + registry[i].roll + ")</option>";
  }

  html += R"(
        </select>
      </div>
      
      <div class='student'>
        <h3>Permission Type:</h3>
        <select name='type' required onchange='updateDuration(this.value)'>
          <option value=''>-- Choose Type --</option>
          <option value='period'>Single Period</option>
          <option value='halfday'>Half Day</option>
          <option value='fullday'>Full Day</option>
          <option value='custom'>Custom Duration</option>
        </select>
      </div>
      
      <div class='student' id='customTime' style='display:none;'>
        <h3>Duration (minutes):</h3>
        <input type='number' name='duration' id='durationInput' min='1' max='480' value='60'>
      </div>
      
      <button type='submit'>✓ Grant Permission</button>
    </form>
    
    <hr style='margin: 30px 0;'>
    <h3>Current Status:</h3>
)";

  for (int i = 0; i < REG_COUNT; i++) {
    html += "<div class='student'><strong>" + registry[i].name + ":</strong> ";
    if (registry[i].onDuty) {
      html += "<span style='color: green;'>✓ ON DUTY</span>";
    } else {
      html += "<span style='color: gray;'>Normal</span>";
    }
    html += "</div>";
  }

  html += R"(
  </div>
  <script>
    function updateDuration(type) {
      const customDiv = document.getElementById('customTime');
      const input = document.getElementById('durationInput');
      if (type === 'custom') {
        customDiv.style.display = 'block';
      } else {
        customDiv.style.display = 'none';
        if (type === 'period') input.value = '120';
        else if (type === 'halfday') input.value = '240';
        else if (type === 'fullday') input.value = '480';
      }
    }=== ESP32 BLE Attendance with RTC ===");

  // Setup RTC
  setupRTC();
  
  // Connect WiFi
  connectWiFi();
  
  // Setup Web Server
  if (wifiReady) {
    server.on("/", handleRoot);
    server.on("/grant", HTTP_POST, handleGrant);
    server.begin();
    Serial.println("Web server started at: http://" + WiFi.localIP().toString());
  // Handle web server requests
  server.handleClient();
  
  // Check if 1 minute has passed since last scan
  static unsigned long lastScan = 0;
  if (millis() - lastScan >= SCAN_INTERVAL) {
    lastScan = millis();
    
    Serial.printf("\n=== Scan #%lu at %s ===\n", millis()/1000, getTimeStr().c_str());
    Serial.printf("Current Period: %d\n", getCurrentPeriod());
    
    // Reset detection tracking
    detectedCount = 0;
    
    // Perform BLE scan
    pBLEScan->start(SCAN_DURATION, false);
    pBLEScan->clearResults();
    
    // Process attendance for all students
    for (int i = 0; i < REG_COUNT; i++) {
      bool beaconDetected = false;
      
      // Check if beacon was detected in this scan
      for (int j = 0; j < detectedCount; j++) {
        if (detected[j] == registry[i].mac) {
          beaconDetected = true;
          break;
        }
      }
      
      // Check on-duty status
      bool onDuty = isOnDutyActive(registry[i]);
      
      // Determine if present
      bool isPresent = beaconDetected || onDuty;
      
      // Send attendance data
      String statusCode = String(registry[i].id) + (isPresent ? "P" : "A");
      Serial.printf("Student %d (%s): %s %s\n", 
                    registry[i].id, 
                    registry[i].name.c_str(), 
                    statusCode.c_str(),
                    onDuty ? "[ON-DUTY]" : "");
      
      sendAttendance(registry[i], isPresent, onDuty);
    }
    
    Serial.println("=== Scan complete ===");
  }
  
  delay(100); // Small delay to prevent watchdog issuesedDeviceCallbacks(new AdvCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(80);
  
  Serial.println("System ready!"
  if (server.hasArg("studentId") && server.hasArg("type")) {
    int studentId = server.arg("studentId").toInt();
    String type = server.arg("type");
    int duration = server.arg("duration").toInt();
    
    for (int i = 0; i < REG_COUNT; i++) {
      if (registry[i].id == studentId) {
        DateTime now = rtc.now();
        registry[i].onDuty = true;
        registry[i].onDutyUntil = now.unixtime() + (duration * 60);
        
        Serial.printf("Permission granted: %s for %d minutes\n", registry[i].name.c_str(), duration);
        
        String html = "<html><body style='font-family:Arial;text-align:center;padding:50px;'>";
        html += "<h1 style='color:green;'>✓ Permission Granted</h1>";
        html += "<p>" + registry[i].name + " marked on-duty for " + String(duration) + " minutes</p>";
        html += "<a href='/' style='display:inline-block;margin-top:20px;padding:10px 20px;background:#667eea;color:white;text-decoration:none;border-radius:5px;'>Back</a>";
        html += "</body></html>";
        
        server.send(200, "text/html", html);
        return;
      }
    }
  }
  server.send(400, "text/html", "<h1>Error: Invalid request</h1><a href='/'>Back</a>");
}

// ---------- BLE CALLBACK ----------
class AdvCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    String mac = dev.getAddress().toString().c_str();
    mac.toUpperCase();
    int rssi = dev.getRSSI();
    if (rssi < RSSI_THRESHOLD) return;

    for (int i = 0; i < REG_COUNT; i++) {
      if (mac == registry[i].mac) {
        bool seen = false;
        for (int j = 0; j < detectedCount; j++) if (detected[j] == mac) seen = true;
        if (!seen && detectedCount < 64) {
          detected[detectedCount++] = mac;
          Serial.printf("Beacon detected: %s (ID:%d) RSSI:%d\n", registry[i].name.c_str(), registry[i].id
DateTime lastScanTime;

// ---------- WIFI ----------
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  wifiReady = WiFi.status() == WL_CONNECTED;
  Serial.println(wifiReady ? "\nWiFi connected" : "\nWiFi failed");
}

// ---------- TIME HELPERS ----------
String getDateStr() {
  // TODO: sync NTP for real time
  return "02-01-2026"; // placeholder
}
String getTimeStr() {
  return "09:00 AM"; // placeholder
}

// ---------- SEND ATTENDANCE ----------
void sendAttendance(const StudentBeacon& s, int rssi) {
  if (!wifiReady || WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not ready, skip send");
    return;
  }
  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["name"] = s.name;
  doc["rollNumber"] = s.roll;
  doc["year"] = s.year;
  doc["department"] = DEPARTMENT;
  doc["location"] = LOCATION_ID;
  doc["rssi"] = rssi;
  doc["date"] = getDateStr();
  doc["time"] = getTimeStr();

  String body;
  serializeJson(doc, body);
  int code = http.POST(body);
  Serial.printf("POST %d: %s\n", code, body.c_str());
  http.end();
}

// ---------- BLE CALLBACK ----------
class AdvCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    String mac = dev.getAddress().toString().c_str();
    mac.toUpperCase();
    int rssi = dev.getRSSI();
    if (rssi < RSSI_THRESHOLD) return;

    for (int i = 0; i < REG_COUNT; i++) {
      if (mac == registry[i].mac) {
        bool seen = false;
        for (int j = 0; j < detectedCount; j++) if (detected[j] == mac) seen = true;
        if (!seen && detectedCount < 64) {
          detected[detectedCount++] = mac;
          Serial.printf("Detected %s (%s) RSSI:%d\n", registry[i].name.c_str(), registry[i].roll.c_str(), rssi);
          sendAttendance(registry[i], rssi);
        }
        break;
      }
    }
  }
};

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 BLE Attendance Scanner");

  connectWiFi();

  BLEDevice::init("ESP32_Attendance");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(80);
}

// ---------- LOOP ----------
void loop() {
  Serial.printf("\nScan start (%d s)\n", SCAN_DURATION);
  detectedCount = 0;
  pBLEScan->start(SCAN_DURATION, false);
  pBLEScan->clearResults();
  Serial.printf("Scan done, found %d student(s)\n", detectedCount);
  delay(SCAN_INTERVAL);
}
