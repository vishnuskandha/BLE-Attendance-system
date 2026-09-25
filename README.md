<div align="center">

# BLE Attendance System

</div>


<!-- README polish: repository metadata badges -->
<p>
  <a href="https://github.com/vishnuskandha/BLE-Attendance-system"><img alt="GitHub stars" src="https://img.shields.io/github/stars/vishnuskandha/BLE-Attendance-system?style=for-the-badge&logo=github&label=Stars"></a>
  <a href="https://github.com/vishnuskandha/BLE-Attendance-system/fork"><img alt="GitHub forks" src="https://img.shields.io/github/forks/vishnuskandha/BLE-Attendance-system?style=for-the-badge&logo=github&label=Forks"></a>
  <a href="https://github.com/vishnuskandha/BLE-Attendance-system/issues"><img alt="GitHub issues" src="https://img.shields.io/github/issues/vishnuskandha/BLE-Attendance-system?style=for-the-badge&logo=github&label=Issues"></a>
  <a href="https://github.com/vishnuskandha/BLE-Attendance-system/commits"><img alt="Last commit" src="https://img.shields.io/github/last-commit/vishnuskandha/BLE-Attendance-system?style=for-the-badge&logo=git&label=Updated"></a>
</p>
<!-- End README polish -->

![CI](https://github.com/vishnuskandha/BLE-Attendance-system/actions/workflows/ci.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)

A complete IoT classroom-attendance system. An **ESP32** scans for student BLE beacons,
resolves them against a student registry, and reports attendance over HTTPS to a
**Vercel serverless API**. The API persists records to **Redis** and serves them to a
responsive **web dashboard** that displays live attendance, filterable reports, and an
optional **AI-powered ID card check** running in the browser.

## System Architecture

```
+-------------+   HTTPS POST   +------------------+   JSON GET   +-----------------+
|   ESP32     |--------------->|  Vercel Backend  |<-------------|  Web Dashboard  |
| BLE scanner |  /api/attendance | api/attendance.js|              |   index.html   |
| + DS3231 RTC|               +------------------+              | + AI ID check   |
+-------------+                            |                     +-----------------+
                                     +-----v------+
                                     |    Redis   |
                                     | (KV store) |
                                     +------------+
```

- **ESP32** scans for student BLE beacons, tracks period timing with a DS3231 RTC,
  and posts `{ studentId, code, status, period, ... }` records over HTTPS.
- **Vercel API** (`api/attendance.js`) validates and deduplicates records and stores
  them in Redis with a rolling 14-day retention window.
- **Dashboard** polls the API and renders live status, filters, and printable reports.

## Features

- **ESP32 firmware** - 60-second BLE scans, RSSI filtering, DS3231 RTC with web-based
  time setting, local web portal for on-duty/permission management, and automatic
  present-marking during approved periods.
- **HTTPS reporting** - `WiFiClientSecure` POSTs with BLE/SSL memory management to
  stay within ESP32 heap limits.
- **Smart rules** - "once present, always present" per period, period-aware validation.
- **Persistent storage** - Redis-backed API with duplicate detection and automatic
  14-day rolling retention (plus a 2,000-record safety cap).
- **AI security module** - optional browser-based ID card and person detection via
  Roboflow inference (camera required).
- **Modern dashboard** - glassmorphism UI, live clock, multi-column filtering,
  print-friendly reports, and responsive mobile layout.

## Quick Start

### 1. Deploy the API to Vercel

1. Push this repository to GitHub.
2. Import it at [vercel.com](https://vercel.com) (Node.js preset, framework: Other).
3. Create a Redis database and set its connection string in the project's
   **Environment Variables** as `REDIS_URL` (or `KV_URL`). `api/attendance.js` reads
   one of these at runtime - no code changes needed.
4. Note your deployment URL, e.g. `https://your-project.vercel.app`.

The `vercel.json` rewrite also serves `index.html` from the same deployment, so the
dashboard is live immediately. A GitHub Pages workflow is included as an alternative
static host for the frontend.

### 2. Flash the ESP32

1. Open `esp32_attendance_optimized.ino` in the Arduino IDE.
2. Set `WIFI_SSID`, `WIFI_PASSWORD`, and `SERVER_URL` (point it at your Vercel
   deployment, e.g. `https://your-project.vercel.app/api/attendance`).
3. Flash the board and open the Serial Monitor at 115200 baud.

See [ESP32_SETUP_GUIDE.md](ESP32_SETUP_GUIDE.md) and
[HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) for wiring, component lists, and the full
firmware walkthrough.

### 3. Use the dashboard

1. Open your deployment URL.
2. Sign in with the default credentials below.
3. Attendance should appear automatically as the ESP32 reports it.

> Default login (demo only - change it in `index.html` before public use):
> **Username** `Principal` / **Password** `admin`

## Documentation

Full guides are kept as separate documents to stay maintainable:

| Guide | Contents |
|-------|----------|
| [DEPLOYMENT.md](DEPLOYMENT.md) | Step-by-step Vercel + Redis + GitHub Pages deployment |
| [ESP32_SETUP_GUIDE.md](ESP32_SETUP_GUIDE.md) | ESP32 setup, wiring, and configuration |
| [API_DOCUMENTATION.md](API_DOCUMENTATION.md) | API endpoints, request/response formats, examples |
| [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) | Component list, assembly, troubleshooting |
| [esp32_code_explanation.md](esp32_code_explanation.md) | Line-by-line firmware walkthrough |

## Repository Structure

```
BLE-Attendance-system/
├── api/                              # Vercel serverless functions
│   ├── attendance.js                 # POST/GET attendance records
│   ├── students.js                   # Student registry API
│   └── debug-kv.js                   # Env/KV diagnostics (disabled in production)
├── scripts/
│   ├── seed_mock_data.js             # Seed Redis with 10 days of mock records
│   └── clear_db.js                   # Clear the attendance key in Redis
├── index.html                        # Web dashboard (+ AI security module)
├── esp32_attendance_optimized.ino    # ESP32 firmware
├── vercel.json                       # Vercel rewrites + CORS headers
├── package.json                      # npm scripts and dependencies
└── docs as listed above              # Deployment and hardware guides
```

## API Overview

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/attendance` | POST | ESP32 reports attendance; returns `recordId` |
| `/api/attendance` | GET | Fetch records, filterable by `date`, `studentId`, `irregularities` |
| `/api/students` | GET | List registered students |

See [API_DOCUMENTATION.md](API_DOCUMENTATION.md) for full request/response examples.

## Development

```bash
npm ci
npm run validate   # Syntax-checks every serverless function
npm start          # vercel dev - local API + static frontend
npm run deploy     # vercel --prod
```

The CI workflow runs `npm ci` and `npm run validate` on every push and pull request.

## Security

Default credentials and the client-side Roboflow publishable key are demo values.
Before a public deployment: change the dashboard login, protect the API with an API
key (see [DEPLOYMENT.md](DEPLOYMENT.md)), and keep `REDIS_URL` in Vercel environment
variables. See [SECURITY.md](SECURITY.md) for details.

## License

MIT - see [LICENSE](LICENSE).
