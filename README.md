# BLE Attendance System

🎓 A complete IoT attendance tracking system using ESP32, BLE beacons, and cloud deployment.

## 🌟 Features

- **Automatic BLE Scanning** - ESP32 scans for student beacons every minute
- **Real-time Tracking** - Live attendance updates via cloud API
- **Staff Permissions** - Local web interface for on-duty/permission management
- **Smart Detection** - Auto-marks present during approved absence periods
- **Modern Web Interface** - Responsive dashboard with filters and reports
- **Cloud-Powered** - Serverless backend on Vercel, frontend on GitHub Pages

## 🏗️ System Architecture

```
┌─────────────┐       ┌─────────────┐       ┌─────────────┐
│   ESP32     │ POST  │   Vercel    │  GET  │   Website   │
│   Scanner   │──────>│   Backend   │<──────│(GitHub Pages)│
│   + RTC     │ Data  │   API       │ Data  │             │
└─────────────┘       └─────────────┘       └─────────────┘
                      Serverless Functions
```

## 📦 Repository Structure

```
BLE-Attendance-system/
├── api/
│   ├── attendance.js       # Vercel serverless function (POST/GET attendance)
│   └── students.js         # Students list API
├── backup/
│   └── index_mordern.html  # Backup of website
├── .github/
│   └── copilot-instructions.md  # AI coding guidelines
├── index.html              # Main web application
├── esp32_ble_scanner.ino   # ESP32 firmware
├── vercel.json             # Vercel configuration
├── package.json            # Node.js dependencies
├── DEPLOYMENT.md           # Complete deployment guide
├── ESP32_SETUP_GUIDE.md    # Hardware setup instructions
├── API_DOCUMENTATION.md    # API reference
├── HARDWARE_GUIDE.md       # Hardware assembly guide
└── README.md               # This file
```

## 🚀 Quick Start

### 1. Deploy to Vercel
**Note the URL**: `https://your-project.vercel.app`

### 2. Configure & Upload ESP32

1. Update `esp32_ble_scanner.ino`:
   ```cpp
   const char* WIFI_SSID = "Your-WiFi";
   const char* WIFI_PASSWORD = "Your-Password";
   const char* SERVER_URL = "https://your-project.vercel.app/api/attendance";
   ```

2. Upload to ESP32 via Arduino IDE

## 📚 Complete Documentation

| Guide | Description |
|-------|-------------|
| **[DEPLOYMENT.md](DEPLOYMENT.md)** | Complete deployment instructions for Vercel & GitHub Pages |
| **[ESP32_SETUP_GUIDE.md](ESP32_SETUP_GUIDE.md)** | ESP32 hardware setup, wiring, and configuration |
| **[API_DOCUMENTATION.md](API_DOCUMENTATION.md)** | API endpoints, request/response formats, examples |
| **[HARDWARE_GUIDE.md](HARDWARE_GUIDE.md)** | Component list, assembly, troubleshooting |

## 🔧 Hardware Requirements

- **ESP32 DevKit V1** - Main controller ($5-10)
- **DS3231 RTC Module** - Real-time clock ($2-5)
- **BLE Beacons** (2+) - Student ID tags ($3-8 each)
- **Jumper Wires** - Connections ($1-2)

**Total Cost**: ~$20-40

## 🎯 Key Features

### ESP32 Firmware
- ✅ 1-minute scan intervals
- ✅ DS3231 RTC integration
- ✅ Local web server for permissions
- ✅ Auto-present during on-duty time
- ✅ Student ID coding (1P, 1A, 2P, 2A)
- ✅ HTTP POST to cloud API

### Web Application
- ✅ Modern glassmorphism design
- ✅ Real-time clock display
- ✅ Multi-column filtering
- ✅ Print-friendly reports
- ✅ Responsive mobile layout
- ✅ API integration with auto-refresh

### Vercel Backend
- ✅ Serverless architecture
- ✅ CORS-enabled endpoints
- ✅ JSON data storage
- ✅ Query filtering (date, student, period)
- ✅ Fast global CDN

## 👥 Student Registry

| ID | Name | Roll Number | Beacon MAC |
|----|------|-------------|------------|
| 1 | Mathumitha R | 310622205081 | 0E:A5:25:A0:00:16 |
| 2 | Lipsa Sahoo | 310622205075 | 0E:A5:25:A0:00:13 |

**Department**: IT-B | **Year**: 4

## 📊 Period Timings

| Period | Time Slot |
|--------|-----------|
| Period 1 | 8:15 AM - 10:15 AM |
| Period 2 | 10:30 AM - 12:45 PM |
| Period 3 | 1:30 PM - 3:45 PM |

## 🔐 Default Credentials

**Web Portal**:
- Username: `Principal`
- Password: `admin`

⚠️ **Change these in production!**

## 🌐 Live URLs (After Deployment)

- **Backend API**: `https://your-project.vercel.app/api/attendance`
- **Website**: `https://your-username.github.io/BLE-Attendance-system/`
- **ESP32 Web Interface**: `http://[ESP32-IP-ADDRESS]`

## 🧪 Testing

### Test API
```bash
# Get all attendance
curl https://your-project.vercel.app/api/attendance

# Get students
curl https://your-project.vercel.app/api/students

# Post test data
curl -X POST https://your-project.vercel.app/api/attendance \
  -H "Content-Type: application/json" \
  -d '{"studentId":1,"code":"1P","status":"Present",...}'
```

### Test Website
1. Open GitHub Pages URL
2. Login with `Principal` / `admin`
3. Navigate to Attendance section
4. Data should load from API

### Test ESP32
1. Power on ESP32
2. Check Serial Monitor (115200 baud)
3. Look for: `✓ Attendance sent: 1P`

## 🛠️ Troubleshooting

### Website shows "No data"
- ✅ Check API URL in `index.html` line ~1050
- ✅ Open browser Console (F12) for errors
- ✅ Verify CORS settings in Vercel

### ESP32 not posting
- ✅ Check WiFi credentials
- ✅ Verify SERVER_URL matches Vercel deployment
- ✅ Test API manually with curl

### Beacons not detected
- ✅ Check beacon batteries
- ✅ Verify MAC addresses
- ✅ Increase RSSI threshold to -90

## 📈 Future Enhancements

- [ ] Database persistence (Vercel KV / MongoDB)
- [ ] Email notifications for absences
- [ ] Real-time WebSocket updates
- [ ] Mobile app (React Native)
- [ ] Export to Excel/PDF
- [ ] Multi-classroom support
- [ ] Facial recognition backup

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

## 📄 License

MIT License - See LICENSE file for details

## 👨‍💻 Tech Stack

- **Frontend**: HTML5, TailwindCSS, Vanilla JavaScript
- **Backend**: Node.js, Vercel Serverless Functions
- **Hardware**: ESP32 (Arduino), DS3231 RTC, BLE Beacons
- **Deployment**: Vercel (API) + GitHub Pages (Frontend)

## 📞 Support

For issues and questions:
- 📖 Check [DEPLOYMENT.md](DEPLOYMENT.md) for deployment help
- 🔧 Check [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) for hardware issues
- 📡 Check [API_DOCUMENTATION.md](API_DOCUMENTATION.md) for API errors
- 🐛 Open an issue on GitHub

---

**Made with ❤️ for Smart Campus Management**

**Last Updated**: January 2, 2026 | **Version**: 1.0.0
