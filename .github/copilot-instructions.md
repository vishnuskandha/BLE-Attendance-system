# BLE Attendance System - AI Coding Instructions

## Project Overview
This is a **single-page attendance management web application** for academic institutions. It uses BLE (Bluetooth Low Energy) technology to track student presence and includes an **AI-powered Security module** for real-time ID card detection via webcam.

## Architecture & Key Patterns

### Single-Page Application (SPA) Design
- **Routing**: Hash-based navigation with `location.hash` and `hashchange` events (see `showSection()`)
- **Sections**: Five main views: `section-login`, `section-dashboard`, `section-attendance`, `section-irregular`, `section-security`
- **Lazy initialization**: Sections init filters only when first displayed (flags: `attendanceInitialized`, `irregularInitialized`)
- **Navigation**: Tab-based nav buttons with `data-target` attributes trigger `showSection(key)`

### Authentication
- **Simple validation**: Hardcoded credentials in script (`validUser = "Principal"`, `validPass = "admin"`)
- **Session handling**: Login hides `section-login` and shows `app-container`; logout reverses this
- **No persistence**: Credentials are in-memory only

### Data & Filtering
- **In-memory tables**: All data is static HTML `<table>` elements with no backend
- **Attendance table**: Columns are Date, Name, Dept, ID, Year, Period1-4 (P/A values)
- **Irregularities table**: Date, Name, Dept, ID, Year, Time, Image
- **Filter architecture differs by section**:
  - **Attendance**: Multi-select checkboxes per column with custom `activeFilters` object
  - **Irregularities**: Simple dropdown select per column (one value at a time)

### 🆕 AI Security Module (v2.0)
- **Location**: `section-security` (lines ~990-1200 in index.html)
- **AI Model**: Roboflow `id-card-detection-enon5` via `inference.js`
- **Detection Classes**: `person`, `idcard` (both detected, logic uses combination)
- **Key Variables**:
  - `model` - Roboflow model instance
  - `video`, `overlay`, `ctx` - Video element and canvas for drawing
  - `detectionBuffer` - Rolling 8-frame buffer for stabilization
  - `stableIDDetected` - Smoothed detection state
- **FPS Throttling**: 10 FPS (100ms frame interval) for performance

#### Detection Logic Flow
```
Person + ID Card → ✅ Verified (green box)
Person only      → ⚠️ Violation (tracks 3s, then captures)
No person        → 🔵 Scanning...
```

#### Key Functions
- `initSecurity()` - Loads Roboflow model with API key
- `startCamera()` - Requests camera access and starts detection loop
- `detectFrame(timestamp)` - Main detection loop (FPS-throttled)
- `updateAIStatus(colorClass, text)` - Updates status indicator
- `captureViolation()` - Captures screenshot when violation detected

### Styling Conventions
- **Tailwind CSS**: All styling via CDN and utility classes
- **Print-friendly**: `onclick="window.print()"` for reports
- **Responsive**: Flexbox layouts with `sm:` breakpoints

## External Dependencies & Integration Points

- **Tailwind CSS CDN**: https://cdn.tailwindcss.com
- **TensorFlow.js (COCO-SSD)**: https://cdn.jsdelivr.net/npm/@tensorflow/tfjs
- **Roboflow Inference**: https://cdn.roboflow.com/0.2.26/roboflow.js
- **OpenCV.js**: https://docs.opencv.org/4.x/opencv.js (async loaded)
- **Roboflow API Key**: `rf_oHASlDufhOSjOs7oiFJKNKtcyqH2`
- **Model**: `id-card-detection-enon5` from jays-workspace

### API Endpoints (Vercel)
- `POST /api/attendance` - Record attendance from ESP32
- `GET /api/attendance` - Fetch attendance records
- `GET /api/students` - Get student list

## Critical Development Workflows

### Modifying AI Detection
1. **Change detection model**: Update `model` name in `initSecurity()`
2. **Adjust confidence**: Modify threshold in detection loop (currently 0.5)
3. **Add new detection classes**: Update class filtering in `detectFrame()`
4. **Tune stabilization**: Adjust `DETECTION_BUFFER_SIZE` and hysteresis thresholds

### Adding Table Data
- Insert new `<tr>` rows directly into `<tbody>` (static HTML approach)
- Ensure column count matches headers and existing filter logic

### Extending Filters
- **Attendance filters**: Add new `<th>` with filter-toggle button and dropdown
- **Irregularities filters**: Auto-generated in `addIrrFilters()`

### Adding New Sections
1. Create new `<section id="section-{name}">` inside `app-container`
2. Add to `sectionMap` object in navigation logic
3. Add nav button with `data-target="{name}"`
4. Initialize lazy logic in `showSection()` if needed

## Project-Specific Conventions

### Naming & IDs
- Element IDs use kebab-case: `section-login`, `attendanceTable`, `webcam`
- Data attributes: `data-target`, `data-col` (column index)
- CSS classes: Tailwind utilities + custom filter classes

### Event Delegation
- Click handlers use `document.addEventListener('click')` to avoid bubbling
- Filter buttons stop propagation: `e.stopPropagation()`

## Key Files & Their Responsibilities

- [index.html](../index.html): Complete application (~1600 lines) - all HTML, CSS, JavaScript
  - Lines 1-120: Meta tags, CDNs (Tailwind, TensorFlow, Roboflow, OpenCV), custom CSS
  - Lines 120-500: Login, Dashboard sections
  - Lines 500-900: Attendance & Irregularities tables with filters
  - Lines 900-1200: 🆕 Security section with AI detection logic
  - Lines 1200+: Navigation handlers, API integration

- [api/](../api/): Serverless backend functions
  - `attendance.js`: Handles POST/GET attendance data
  - `students.js`: Handles student data

- [README.md](../README.md): Full documentation with AI Security features
- [DEPLOYMENT.md](../DEPLOYMENT.md): Deployment instructions (GitHub Pages + Vercel)

---

**Last Updated**: January 31, 2026
**Version**: 2.0.0 (with AI Security)
**Deployment**: GitHub Pages (Frontend) + Vercel (Backend)
