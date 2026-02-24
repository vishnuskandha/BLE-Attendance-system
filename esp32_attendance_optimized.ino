/*
 * ESP32 BLE Attendance Scanner with RTC & Web Interface (OPTIMIZED)
 * Version: 3.0.0 (Unified Update)
 * Reduced memory footprint for compilation
 */

#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <RTClib.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

// ---------- CONFIGURE THESE ----------
const char *WIFI_SSID = "Madhu";
const char *WIFI_PASSWORD = "12345678";
const char *SERVER_URL =
    "https://ble-attendance-system-pink.vercel.app/api/attendance";
const char *LOCATION_ID = "CLASSROOM_A";
const char *DEPARTMENT = "IT-B";

#define RTC_SDA 21
#define RTC_SCL 22
const int SCAN_DURATION = 5;
const int SCAN_INTERVAL = 60000;
const int RSSI_THRESHOLD = -80;

// ---------- STUDENT REGISTRY ----------
struct StudentBeacon {
  int id;
  String mac;
  String name;
  String roll;
  int year;
  bool onDuty;
  unsigned long onDutyUntil;
};

StudentBeacon registry[] = {
    {1, "0E:A5:25:A0:00:16", "Mathumitha R", "310622205081", 4, false, 0},
    {2, "0E:A5:25:A0:00:13", "Lipsa Sahoo", "310622205075", 4, false, 0}};
const int REG_COUNT = 2;

struct Period {
  int startHour, startMin, endHour, endMin;
  const char *name;
};

Period periods[] = {{8, 15, 10, 15, "Period 1"},
                    {10, 30, 12, 45, "Period 2"},
                    {13, 30, 15, 45, "Period 3"}};
const int PERIOD_COUNT = 3;

// ---------- GLOBALS ----------
BLEScan *pBLEScan;
RTC_DS3231 rtc;
WebServer server(80);
Preferences prefs;
bool wifiReady = false;
bool rtcReady = false;
bool rtcTimeSet = false;
String detected[64];
int detectedCount = 0;

// ---------- HTML TEMPLATES (MINIMAL) ----------
const char HTML_HEADER[] PROGMEM =
    "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' "
    "content='width=device-width,initial-scale=1'>"
    "<style>body{font-family:Arial;padding:20px;background:#667eea;margin:0}"
    ".c{max-width:600px;margin:0 "
    "auto;background:#fff;padding:20px;border-radius:10px}"
    "h1{color:#667eea;text-align:center;font-size:24px}"
    ".s{background:#f9f9f9;padding:12px;margin:8px "
    "0;border-radius:5px;border-left:3px solid #667eea}"
    "select,input{width:100%;padding:10px;margin:5px 0;border:2px solid "
    "#ddd;border-radius:5px;box-sizing:border-box}"
    "button{background:#667eea;color:#fff;padding:12px;border:none;border-"
    "radius:5px;cursor:pointer;width:100%;margin-top:10px}"
    "button:hover{opacity:0.9}.w{background:#fef3c7;padding:12px;margin:10px "
    "0;border-radius:5px;border-left:3px solid #f59e0b}"
    ".on{color:#10b981;font-weight:bold}.off{color:#6b7280}a{color:#667eea;"
    "text-decoration:none}"
    "</style></head><body><div class='c'>";

// ---------- RTC SETUP ----------
void setupRTC() {
  Wire.begin(RTC_SDA, RTC_SCL);
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    rtcReady = false;
    return;
  }

  rtcReady = true;
  Serial.println("RTC OK");

  if (rtc.lostPower() || rtc.now().year() < 2020) {
    Serial.println("RTC needs time setup!");
    rtcTimeSet = false;
  } else {
    rtcTimeSet = true;
    DateTime now = rtc.now();
    Serial.printf("RTC: %02d-%02d-%04d %02d:%02d\n", now.day(), now.month(),
                  now.year(), now.hour(), now.minute());
  }

  prefs.begin("rtc", false);
  if (rtcTimeSet)
    prefs.putBool("set", true);
  prefs.end();
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi");
  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  wifiReady = WiFi.status() == WL_CONNECTED;
  if (wifiReady) {
    Serial.println("\nWiFi OK");
    Serial.println(WiFi.localIP());

    // --- DEBUG CONNECTIVITY ---
    Serial.println("Testing connectivity...");
    configTime(0, 0, "pool.ntp.org"); // Basic time sync to test internet
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time - Internet Check Failed?");
    } else {
      Serial.println("Time synced - Internet OK");
    }
    // --- END DEBUG ---
  }
}

String getDateStr() {
  if (!rtcReady || !rtcTimeSet)
    return "NOT_SET";
  DateTime now = rtc.now();
  char buf[16];
  sprintf(buf, "%02d-%02d-%04d", now.day(), now.month(), now.year());
  return String(buf);
}

String getTimeStr() {
  if (!rtcReady || !rtcTimeSet)
    return "NOT_SET";
  DateTime now = rtc.now();
  int h = now.hour();
  String ampm = h >= 12 ? "PM" : "AM";
  if (h > 12)
    h -= 12;
  if (h == 0)
    h = 12;
  char buf[16];
  sprintf(buf, "%02d:%02d %s", h, now.minute(), ampm.c_str());
  return String(buf);
}

int getCurrentPeriod() {
  if (!rtcReady || !rtcTimeSet)
    return 0;
  DateTime now = rtc.now();
  int currentMin = now.hour() * 60 + now.minute();

  for (int i = 0; i < PERIOD_COUNT; i++) {
    int startMin = periods[i].startHour * 60 + periods[i].startMin;
    int endMin = periods[i].endHour * 60 + periods[i].endMin;
    if (currentMin >= startMin && currentMin < endMin)
      return i + 1;
  }
  return 0;
}

bool isOnDutyActive(StudentBeacon &student) {
  if (!student.onDuty || !rtcReady || !rtcTimeSet)
    return false;

  DateTime now = rtc.now();
  if (now.unixtime() > student.onDutyUntil) {
    student.onDuty = false;
    student.onDutyUntil = 0;
    return false;
  }
  return true;
}

void sendAttendance(const StudentBeacon &s, bool present, bool onDuty) {
  if (!rtcTimeSet || WiFi.status() != WL_CONNECTED)
    return;

  StaticJsonDocument<512> doc;
  doc["studentId"] = s.id;
  doc["code"] = String(s.id) + (present ? "P" : "A");
  doc["name"] = s.name;
  doc["rollNumber"] = s.roll;
  doc["year"] = s.year;
  doc["department"] = DEPARTMENT;
  doc["location"] = LOCATION_ID;
  doc["status"] = present ? "Present" : "Absent";
  doc["onDuty"] = onDuty;
  doc["period"] = getCurrentPeriod();
  doc["date"] = getDateStr();
  doc["time"] = getTimeStr();

  String body;
  serializeJson(doc, body);

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15);

  HTTPClient http;
  http.setTimeout(15000);
  if (!http.begin(client, SERVER_URL)) {
    Serial.printf("  begin() failed: %s\n", s.name.c_str());
    return;
  }
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);
  if (code > 0) {
    Serial.printf("  POST OK (%d): %s\n", code, s.name.c_str());
  } else {
    Serial.printf("  POST FAIL (%s): %s\n", http.errorToString(code).c_str(),
                  s.name.c_str());
  }
  http.end();
}

// ---------- WEB HANDLERS ----------
void handleRoot() {
  String html = FPSTR(HTML_HEADER);
  html += "<h1>Attendance Portal</h1>";

  if (!rtcTimeSet) {
    html += "<div class='w'><strong>RTC Time Not Set!</strong><br>";
    html += "<a href='/settime'><button>Set Time Now</button></a></div>";
  }

  html += "<p style='text-align:center;font-size:14px'>Time: " + getTimeStr() +
          " | " + getDateStr() + "</p>";
  html += "<form method='POST' action='/grant'><div "
          "class='s'><h3>Student:</h3><select name='id' required>";
  html += "<option value=''>-- Select --</option>";

  for (int i = 0; i < REG_COUNT; i++) {
    html += "<option value='" + String(registry[i].id) + "'>" +
            registry[i].name + "</option>";
  }

  html += "</select></div><div class='s'><h3>Permission:</h3>";
  html += "<select name='type' required><option value=''>-- Select --</option>";
  html += "<option value='120'>Single Period (2h)</option>";
  html += "<option value='240'>Half Day (4h)</option>";
  html += "<option value='480'>Full Day (8h)</option></select></div>";
  html += "<button type='submit'>Grant Permission</button></form><hr>";
  html += "<h3>Status:</h3>";

  for (int i = 0; i < REG_COUNT; i++) {
    html += "<div class='s'><strong>" + registry[i].name + ":</strong> ";
    html += registry[i].onDuty ? "<span class='on'>ON DUTY</span>"
                               : "<span class='off'>Normal</span>";
    html += "</div>";
  }

  html += "<a href='/settime'><button>Configure Time</button></a>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSetTimePage() {
  String currentDate = "2026-02-03", currentTime = "09:00";

  if (rtcReady) {
    DateTime now = rtc.now();
    char dateBuf[16], timeBuf[16];
    sprintf(dateBuf, "%04d-%02d-%02d", now.year(), now.month(), now.day());
    sprintf(timeBuf, "%02d:%02d", now.hour(), now.minute());
    currentDate = String(dateBuf);
    currentTime = String(timeBuf);
  }

  String html = FPSTR(HTML_HEADER);
  html += "<h1>Set RTC Time</h1>";
  html +=
      "<div class='w'>Current: " + getDateStr() + " " + getTimeStr() + "</div>";
  html += "<form method='POST' action='/savetime'>";
  html += "<div class='s'><label>Date:</label><input type='date' name='date' "
          "value='" +
          currentDate + "' required></div>";
  html += "<div class='s'><label>Time (24h):</label><input type='time' "
          "name='time' value='" +
          currentTime + "' required></div>";
  html += "<button type='submit'>Save to RTC</button>";
  html += "<a href='/'><button type='button' "
          "style='background:#6b7280;margin-top:5px'>Back</button></a></form>";
  html += "<p style='font-size:12px;color:#666;margin-top:15px'>Note: RTC has "
          "backup battery for persistent time.</p>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSaveTime() {
  if (!rtcReady) {
    server.send(500, "text/html",
                "<h1>Error: RTC not available</h1><a href='/'>Back</a>");
    return;
  }

  if (server.hasArg("date") && server.hasArg("time")) {
    String dateStr = server.arg("date");
    String timeStr = server.arg("time");

    int year = dateStr.substring(0, 4).toInt();
    int month = dateStr.substring(5, 7).toInt();
    int day = dateStr.substring(8, 10).toInt();
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();

    rtc.adjust(DateTime(year, month, day, hour, minute, 0));

    prefs.begin("rtc", false);
    prefs.putBool("set", true);
    prefs.end();

    rtcTimeSet = true;

    Serial.printf("RTC set: %04d-%02d-%02d %02d:%02d\n", year, month, day, hour,
                  minute);

    String html = FPSTR(HTML_HEADER);
    html += "<h1 style='color:#10b981'>Time Saved!</h1>";
    html += "<div class='s' style='text-align:center'><h2>" + getDateStr() +
            "</h2><h2>" + getTimeStr() + "</h2></div>";
    html += "<p style='text-align:center'>RTC will maintain time even when "
            "powered off.</p>";
    html += "<a href='/'><button>Back to Portal</button></a>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  } else {
    server.send(400, "text/html",
                "<h1>Error</h1><a href='/settime'>Try Again</a>");
  }
}

void handleGrant() {
  if (!rtcTimeSet) {
    server.send(
        400, "text/html",
        "<h1>Error: Set RTC time first</h1><a href='/settime'>Set Time</a>");
    return;
  }

  if (server.hasArg("id") && server.hasArg("type")) {
    int studentId = server.arg("id").toInt();
    int duration = server.arg("type").toInt();

    for (int i = 0; i < REG_COUNT; i++) {
      if (registry[i].id == studentId) {
        DateTime now = rtc.now();
        registry[i].onDuty = true;
        registry[i].onDutyUntil = now.unixtime() + (duration * 60);

        Serial.printf("Permission: %s for %dm\n", registry[i].name.c_str(),
                      duration);

        String html = FPSTR(HTML_HEADER);
        html += "<h1 style='color:#10b981'>Permission Granted</h1>";
        html += "<div class='s' style='text-align:center'><h2>" +
                registry[i].name + "</h2>";
        html += "<p>On-duty for <strong>" + String(duration) +
                " minutes</strong></p></div>";
        html += "<a href='/'><button>Back</button></a></div></body></html>";
        server.send(200, "text/html", html);
        return;
      }
    }
  }
  server.send(400, "text/html", "<h1>Error</h1><a href='/'>Back</a>");
}

// ---------- BLE CALLBACK ----------
class AdvCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {
    String mac = dev.getAddress().toString().c_str();
    mac.toUpperCase();
    int rssi = dev.getRSSI();

    if (rssi < RSSI_THRESHOLD)
      return;

    for (int i = 0; i < REG_COUNT; i++) {
      if (mac == registry[i].mac) {
        bool seen = false;
        for (int j = 0; j < detectedCount; j++) {
          if (detected[j] == mac) {
            seen = true;
            break;
          }
        }
        if (!seen && detectedCount < 64) {
          detected[detectedCount++] = mac;
          Serial.printf("Beacon: %s RSSI:%d\n", registry[i].name.c_str(), rssi);
        }
        break;
      }
    }
  }
};

// Helper to reinitialize BLE after sending
void initBLE() {
  BLEDevice::init("ESP32_Attendance");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(80);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 Attendance v2.0 ===");

  setupRTC();
  connectWiFi();

  if (wifiReady) {
    server.on("/", handleRoot);
    server.on("/grant", HTTP_POST, handleGrant);
    server.on("/settime", handleSetTimePage);
    server.on("/savetime", HTTP_POST, handleSaveTime);
    server.begin();
    Serial.println("Web server started");
    Serial.print("Portal: http://");
    Serial.println(WiFi.localIP());
  }

  initBLE();

  Serial.println("System ready!");
  if (!rtcTimeSet) {
    Serial.println("*** SET TIME FIRST ***");
    Serial.print("Go to: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/settime");
  }
}

// ---------- LOOP ----------
void loop() {
  server.handleClient();

  static unsigned long lastScan = 0;
  if (millis() - lastScan >= SCAN_INTERVAL) {
    lastScan = millis();

    if (!rtcTimeSet) {
      Serial.println("Scan skipped - set RTC time first");
      return;
    }

    Serial.println("\n========== SCAN ==========");
    Serial.printf("%s | Period: %d\n", getTimeStr().c_str(),
                  getCurrentPeriod());

    // STEP 1: Scan for BLE beacons
    detectedCount = 0;
    pBLEScan->start(SCAN_DURATION, false);
    pBLEScan->clearResults();
    Serial.printf("Found %d beacon(s)\n", detectedCount);

    // STEP 2: Collect results while BLE is still active
    bool present[REG_COUNT];
    bool duty[REG_COUNT];
    for (int i = 0; i < REG_COUNT; i++) {
      bool beaconDetected = false;
      for (int j = 0; j < detectedCount; j++) {
        if (detected[j] == registry[i].mac) {
          beaconDetected = true;
          break;
        }
      }
      duty[i] = isOnDutyActive(registry[i]);
      present[i] = beaconDetected || duty[i];

      String statusCode = String(registry[i].id) + (present[i] ? "P" : "A");
      Serial.printf("[%s] %s: %s", statusCode.c_str(), registry[i].name.c_str(),
                    present[i] ? "PRESENT" : "ABSENT");
      if (duty[i])
        Serial.print(" (ON-DUTY)");
      if (beaconDetected)
        Serial.print(" (BEACON)");
      Serial.println();
    }

    // STEP 3: Shut down BLE to free ~60KB RAM for SSL
    BLEDevice::deinit(true);
    Serial.printf("BLE off - Free heap: %d bytes\n", ESP.getFreeHeap());

    // STEP 4: Send attendance over HTTPS (now we have enough memory)
    for (int i = 0; i < REG_COUNT; i++) {
      sendAttendance(registry[i], present[i], duty[i]);
    }

    // STEP 5: Restart BLE for next scan
    initBLE();
    Serial.printf("BLE on - Free heap: %d bytes\n", ESP.getFreeHeap());

    Serial.println("Next scan in 1 min");
  }

  delay(100);
}
