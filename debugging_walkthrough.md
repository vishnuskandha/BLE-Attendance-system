# ESP32 HTTPS Debugging Walkthrough

**Date:** 13-Feb-2026
**Issue:** ESP32 could not send attendance data to Vercel API — `POST FAILED (connection refused)`

---

## Problem Statement

The ESP32 BLE Attendance System connected to WiFi successfully and could sync NTP time, but all HTTPS POST requests to the Vercel API failed with `connection refused` (HTTP error code -1). Attendance data was never reaching the server.

---

## Debugging Timeline

### Phase 1: Initial Error Identification

**Symptom:** Serial Monitor showed `POST -1` for all students.

**Action:** Added detailed error logging using `http.errorToString(code)`:
```
POST FAILED (connection refused): Mathumitha R
```

This revealed the error was "connection refused" — not a timeout, DNS failure, or HTTP error.

---

### Phase 2: Network Investigation

**Hypothesis 1: Network is blocking Vercel**

Tested by pinging the Vercel URL from the laptop:
```
curl → HTTP 200 ✅ (laptop works)
ESP32 → connection refused ❌
```

**Key Discovery:** The laptop was on `ACT_003F` WiFi, but the ESP32 was on `Madhu` (phone hotspot). The `Madhu` hotspot had no outbound internet for HTTPS — even NTP failed on that network.

**Action:** Switched ESP32 to `ACT_003F` (same network as laptop).

**Result:** NTP now worked ✅, but HTTPS still failed ❌.

---

### Phase 3: SSL vs HTTP Isolation

**Hypothesis 2: ESP32 SSL/TLS implementation is broken**

Added a diagnostic that tested both plain HTTP and HTTPS to httpbin.org:

```
HTTP test (no SSL):   OK (200)  ✅
HTTPS test (with SSL): connection refused (-1) ❌
```

**Conclusion:** The network works fine for HTTP. The problem is specifically with SSL/TLS connections on the ESP32.

---

### Phase 4: Memory Analysis

**Hypothesis 3: Not enough RAM for SSL buffers**

Added free heap monitoring:
```
Free heap: 45,232 bytes
Free heap after BLE stop: 45,204 bytes ← pBLEScan->stop() didn't help
```

**Root Cause Found!** SSL needs ~50KB+ for its TLS handshake buffers, but only **45KB** was free. The BLE subsystem was consuming **~130KB** of RAM.

| Component | Memory Usage |
|---|---|
| BLE subsystem | ~130KB |
| WiFi + TCP | ~40KB |
| WebServer | ~20KB |
| Application | ~30KB |
| **Free** | **~45KB** ← Not enough for SSL (~50KB+) |

Note: `pBLEScan->stop()` only stopped the scan but didn't release BLE memory. We needed `BLEDevice::deinit(true)` to fully tear down the BLE stack.

---

### Phase 5: Solution — BLE Deinit/Reinit Cycle

**Fix:** Completely shut down BLE before HTTPS, then restart it after.

```
BEFORE:  BLE scan → send HTTPS (BLE still running) → fails
AFTER:   BLE scan → deinit BLE → send HTTPS → reinit BLE → works!
```

**Memory after fix:**
```
After BLE deinit:   Free heap: 175,956 bytes ← 175KB! Plenty for SSL
POST OK (200): Mathumitha R  ✅
POST OK (200): Lipsa Sahoo   ✅
After BLE reinit:   Free heap: 174,444 bytes ← BLE restarted
```

---

## Changes Made

### 1. `sendAttendance()` — Simplified
- Removed retry logic (no longer needed — the real issue was memory)
- Uses `WiFiClientSecure` with `setInsecure()` for SSL
- 15-second timeout for both SSL and HTTP

### 2. `initBLE()` — New helper function
- Extracted BLE initialization into a reusable function
- Called from `setup()` and after each HTTPS send cycle
- Placed after `AdvCallbacks` class definition (required for compilation)

### 3. Main `loop()` — 5-Step Cycle
Restructured from interleaved (scan+send per student) to batched:
1. **Scan** — Detect all beacons
2. **Collect** — Store results in arrays
3. **Deinit BLE** — Free ~130KB RAM (`BLEDevice::deinit(true)`)
4. **Send HTTPS** — All students' data sent with ~175KB available
5. **Reinit BLE** — Restart for next cycle

### 4. WiFi Credentials
Changed from `Madhu` hotspot (no internet) to `ACT_003F` (working internet).

---

## Key Lessons Learned

1. **`connection refused` on ESP32 doesn't always mean the server refused** — it can mean the SSL library failed to allocate memory for the handshake.

2. **BLE + SSL cannot coexist on ESP32** — BLE uses ~130KB, SSL needs ~50KB, and the ESP32 only has ~320KB total. Time-sharing is the solution.

3. **`pBLEScan->stop()` ≠ free memory** — It stops scanning but doesn't release BLE memory. Only `BLEDevice::deinit(true)` fully releases it.

4. **Always test HTTP vs HTTPS separately** — This isolation test immediately revealed the issue was SSL-specific, not network-related.

5. **Free heap monitoring is essential** on embedded systems — `ESP.getFreeHeap()` should be your first diagnostic tool for mysterious failures.

---

## Final Working Output
```
=== ESP32 Attendance v2.0 ===
RTC OK
RTC: 13-02-2026 14:42
WiFi...
WiFi OK
192.168.88.24
Testing connectivity...
Time synced - Internet OK
Web server started
Portal: http://192.168.88.24
System ready!

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
