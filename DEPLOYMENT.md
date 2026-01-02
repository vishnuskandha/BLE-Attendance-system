# Deployment Guide - BLE Attendance System

## 📋 Overview

This system requires **two deployments**:
1. **Frontend (Website)** → GitHub Pages (FREE)
2. **Backend (API)** → Vercel (FREE)

```
┌─────────────────────────────────────────────────────────────┐
│                    ARCHITECTURE                              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   ESP32          POST         Vercel API        GET          │
│   Scanner   ──────────────►   (Backend)   ◄──────────────   │
│                               /api/attendance                │
│                                     │                        │
│                                     │                        │
│                                     ▼                        │
│                              GitHub Pages                    │
│                               (Frontend)                     │
│                              index.html                      │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 🚀 Step 1: Deploy Backend to Vercel

### Prerequisites
- GitHub account
- Vercel account (sign up with GitHub)

### Option A: One-Click Deploy

1. Push your code to GitHub
2. Go to [vercel.com](https://vercel.com)
3. Click "New Project"
4. Import your GitHub repository
5. Click "Deploy"
6. Note your URL: `https://your-project-name.vercel.app`

### Option B: CLI Deploy

```bash
# Install Vercel CLI
npm install -g vercel

# Login to Vercel
vercel login

# Deploy (from project root)
vercel

# Deploy to production
vercel --prod
```

### Verify API

Test your deployed API:

```bash
# Get attendance (should return empty array initially)
curl https://your-project-name.vercel.app/api/attendance

# Get students list
curl https://your-project-name.vercel.app/api/students

# Post test attendance
curl -X POST https://your-project-name.vercel.app/api/attendance \
  -H "Content-Type: application/json" \
  -d '{"studentId":1,"code":"1P","name":"Test","status":"Present","date":"02-01-2026"}'
```

---

## 🌐 Step 2: Deploy Frontend to GitHub Pages

### Enable GitHub Pages

1. Go to your GitHub repository
2. Click **Settings** tab
3. Scroll to **Pages** section (left sidebar)
4. Source: **Deploy from a branch**
5. Branch: **main** (or master)
6. Folder: **/ (root)**
7. Click **Save**

### Your Website URL

After a few minutes, your site will be live at:
```
https://YOUR_USERNAME.github.io/BLE-Attendance-system/
```

---

## ⚙️ Step 3: Configure Website API URL

### Update index.html

Open `index.html` and find this section (near line 596):

```javascript
// ============================================
// API CONFIGURATION - UPDATE THIS URL!
// ============================================
const API_BASE_URL = 'https://ble-attendance-api.vercel.app';
```

Replace with your Vercel URL:

```javascript
const API_BASE_URL = 'https://YOUR-PROJECT-NAME.vercel.app';
```

### Commit and Push

```bash
git add index.html
git commit -m "Update API URL"
git push
```

---

## 🔧 Step 4: Configure ESP32

Update `esp32_ble_scanner.ino`:

```cpp
// WiFi credentials
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// API endpoint (your Vercel URL)
const char* SERVER_URL = "https://YOUR-PROJECT-NAME.vercel.app/api/attendance";
```

Upload to ESP32 via Arduino IDE.

---

## ✅ Step 5: Verify Everything Works

### 1. Check ESP32 Serial Monitor
```
✓ WiFi connected
✓ IP: 192.168.1.xxx
✓ Web server started
✓ RTC initialized
Scanning for beacons...
✓ Attendance sent: 1P
```

### 2. Check Vercel Logs
Go to your Vercel dashboard → Project → Logs

### 3. Check Website
1. Open `https://YOUR_USERNAME.github.io/BLE-Attendance-system/`
2. Login with `Principal` / `admin`
3. Go to Attendance section
4. Data should load from API

---

## 🔍 Troubleshooting

### CORS Errors

If you see CORS errors in browser console:

1. Check `vercel.json` has CORS headers
2. Redeploy: `vercel --prod`

### API Not Working

1. Test API directly: `curl https://your-api.vercel.app/api/attendance`
2. Check Vercel function logs
3. Verify `api/attendance.js` is in project root

### ESP32 Can't Connect

1. Check WiFi credentials
2. Verify SERVER_URL matches Vercel deployment
3. Test with `curl` first
4. Check ESP32 is on same network (for testing)

### GitHub Pages Not Updating

1. Check Actions tab for build status
2. Clear browser cache
3. Wait 2-3 minutes after push

---

## 📝 File Structure for Deployment

```
BLE-Attendance-system/
├── api/
│   ├── attendance.js     # ← Vercel will deploy this
│   └── students.js       # ← Vercel will deploy this
├── index.html            # ← GitHub Pages serves this
├── vercel.json           # ← Vercel configuration
├── package.json          # ← Node.js dependencies
├── esp32_ble_scanner.ino # ← Upload to ESP32
└── README.md
```

---

## 🔐 Security Notes

### For Production

1. **Change login credentials** in `index.html`
2. **Add API authentication** (API key or JWT)
3. **Use environment variables** for sensitive data
4. **Enable HTTPS** (automatic on Vercel/GitHub Pages)

### Adding API Key (Optional)

In `api/attendance.js`:
```javascript
const API_KEY = process.env.API_KEY;

if (req.headers['x-api-key'] !== API_KEY) {
  return res.status(401).json({ error: 'Unauthorized' });
}
```

In ESP32:
```cpp
http.addHeader("X-API-Key", "your-secret-key");
```

---

## 📊 Cost Breakdown

| Service | Free Tier | Limits |
|---------|-----------|--------|
| **Vercel** | Yes | 100GB bandwidth, 100K function invocations |
| **GitHub Pages** | Yes | 100GB bandwidth, 1GB storage |

**Total Cost**: $0 for small to medium usage

---

## 🎉 You're Done!

Your system should now be:
- ✅ ESP32 posting attendance every minute
- ✅ Vercel API receiving and storing data
- ✅ GitHub Pages website displaying live data
- ✅ Auto-refresh every 60 seconds

---

**Need Help?**
- Vercel Docs: https://vercel.com/docs
- GitHub Pages Docs: https://docs.github.com/en/pages
- ESP32 Arduino: https://docs.espressif.com/projects/arduino-esp32/

