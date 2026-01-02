# API Documentation - BLE Attendance System

## 📡 Overview

This document describes the HTTP API used by the ESP32 to post attendance data to your server.

---

## 🔗 Endpoint

### POST `/api/attendance`

Receives attendance records from ESP32 scanner.

**URL**: `http://YOUR_SERVER:3000/api/attendance`

**Method**: `POST`

**Content-Type**: `application/json`

---

## 📤 Request Format

### Headers
```http
POST /api/attendance HTTP/1.1
Host: YOUR_SERVER:3000
Content-Type: application/json
Content-Length: [length]
```

### JSON Payload

```json
{
  "studentId": 1,
  "code": "1P",
  "name": "Mathumitha R",
  "rollNumber": "310622205081",
  "year": 4,
  "department": "IT-B",
  "location": "CLASSROOM_A",
  "status": "Present",
  "onDuty": false,
  "period": 2,
  "date": "02-01-2026",
  "time": "10:45 AM"
}
```

### Field Descriptions

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `studentId` | Integer | Unique student identifier (1, 2, 3...) | `1` |
| `code` | String | Status code (format: `[ID][P/A]`) | `"1P"`, `"2A"` |
| `name` | String | Student full name | `"Mathumitha R"` |
| `rollNumber` | String | Student roll number | `"310622205081"` |
| `year` | Integer | Academic year | `4` |
| `department` | String | Department/Section code | `"IT-B"` |
| `location` | String | Room/classroom identifier | `"CLASSROOM_A"` |
| `status` | String | Attendance status | `"Present"` or `"Absent"` |
| `onDuty` | Boolean | On-duty permission flag | `true` or `false` |
| `period` | Integer | Current period number (0 if no period) | `1`, `2`, `3`, `0` |
| `date` | String | Date in DD-MM-YYYY format | `"02-01-2026"` |
| `time` | String | Time in 12-hour format | `"10:45 AM"` |

---

## 📥 Response Format

### Success Response

**Status Code**: `200 OK`

```json
{
  "success": true,
  "message": "Attendance recorded",
  "recordId": "abc123xyz"
}
```

### Error Responses

#### 400 Bad Request
```json
{
  "success": false,
  "error": "Invalid data format"
}
```

#### 500 Internal Server Error
```json
{
  "success": false,
  "error": "Database error"
}
```

---

## 🔢 Status Code System

### Code Format: `[StudentID][Status]`

| Code | Meaning | Description |
|------|---------|-------------|
| **1P** | Student 1 Present | Mathumitha R is present |
| **1A** | Student 1 Absent | Mathumitha R is absent |
| **2P** | Student 2 Present | Lipsa Sahoo is present |
| **2A** | Student 2 Absent | Lipsa Sahoo is absent |

### Logic
```javascript
// Parsing the code
const studentId = parseInt(code[0]);     // Extract ID (1, 2, 3...)
const status = code.substring(1);         // Extract status (P or A)

// Example
"1P" → Student ID: 1, Status: Present
"2A" → Student ID: 2, Status: Absent
```

---

## 📊 Sample Requests

### Example 1: Student Present (Beacon Detected)
```json
POST /api/attendance

{
  "studentId": 1,
  "code": "1P",
  "name": "Mathumitha R",
  "rollNumber": "310622205081",
  "year": 4,
  "department": "IT-B",
  "location": "CLASSROOM_A",
  "status": "Present",
  "onDuty": false,
  "period": 2,
  "date": "02-01-2026",
  "time": "10:45 AM"
}
```

### Example 2: Student Present (On-Duty)
```json
POST /api/attendance

{
  "studentId": 2,
  "code": "2P",
  "name": "Lipsa Sahoo",
  "rollNumber": "310622205075",
  "year": 4,
  "department": "IT-B",
  "location": "CLASSROOM_A",
  "status": "Present",
  "onDuty": true,
  "period": 1,
  "date": "02-01-2026",
  "time": "09:15 AM"
}
```

### Example 3: Student Absent
```json
POST /api/attendance

{
  "studentId": 1,
  "code": "1A",
  "name": "Mathumitha R",
  "rollNumber": "310622205081",
  "year": 4,
  "department": "IT-B",
  "location": "CLASSROOM_A",
  "status": "Absent",
  "onDuty": false,
  "period": 3,
  "date": "02-01-2026",
  "time": "02:00 PM"
}
```

---

## 🖥️ Node.js Server Example

### Simple Express Server

```javascript
const express = require('express');
const cors = require('cors');
const fs = require('fs');

const app = express();
app.use(cors());
app.use(express.json());

// Store attendance records
let attendanceDB = [];

// POST endpoint - Receive attendance from ESP32
app.post('/api/attendance', (req, res) => {
  const record = {
    ...req.body,
    receivedAt: new Date().toISOString(),
    id: Date.now().toString()
  };
  
  // Validate required fields
  if (!record.studentId || !record.code || !record.status) {
    return res.status(400).json({
      success: false,
      error: 'Missing required fields'
    });
  }
  
  // Save to database
  attendanceDB.push(record);
  
  // Log to console
  console.log(`✅ Attendance: ${record.name} - ${record.code} (Period ${record.period})`);
  
  // Optional: Save to file
  fs.writeFileSync('attendance.json', JSON.stringify(attendanceDB, null, 2));
  
  // Send success response
  res.json({
    success: true,
    message: 'Attendance recorded',
    recordId: record.id
  });
});

// GET endpoint - Fetch all attendance records
app.get('/api/attendance', (req, res) => {
  res.json(attendanceDB);
});

// GET endpoint - Fetch today's attendance
app.get('/api/attendance/today', (req, res) => {
  const today = new Date().toLocaleDateString('en-GB').split('/').join('-');
  const todayRecords = attendanceDB.filter(r => r.date === today);
  res.json(todayRecords);
});

// GET endpoint - Fetch by student ID
app.get('/api/attendance/student/:id', (req, res) => {
  const studentId = parseInt(req.params.id);
  const records = attendanceDB.filter(r => r.studentId === studentId);
  res.json(records);
});

const PORT = 3000;
app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Attendance API Server running on http://0.0.0.0:${PORT}`);
  console.log(`📡 ESP32 should POST to: http://YOUR_SERVER_IP:${PORT}/api/attendance`);
});
```

### Installation
```bash
npm install express cors
node server.js
```

---

## 🔒 Security Considerations

### Authentication (Optional)
Add API key authentication:

```javascript
// Server-side
const API_KEY = 'your-secret-key-here';

app.post('/api/attendance', (req, res) => {
  const apiKey = req.headers['x-api-key'];
  
  if (apiKey !== API_KEY) {
    return res.status(401).json({
      success: false,
      error: 'Unauthorized'
    });
  }
  
  // Process attendance...
});
```

```cpp
// ESP32 side
http.addHeader("X-API-Key", "your-secret-key-here");
```

---

## 🧪 Testing the API

### Using cURL
```bash
curl -X POST http://localhost:3000/api/attendance \
  -H "Content-Type: application/json" \
  -d '{
    "studentId": 1,
    "code": "1P",
    "name": "Mathumitha R",
    "rollNumber": "310622205081",
    "year": 4,
    "department": "IT-B",
    "location": "CLASSROOM_A",
    "status": "Present",
    "onDuty": false,
    "period": 2,
    "date": "02-01-2026",
    "time": "10:45 AM"
  }'
```

### Using Postman
1. Create new request
2. Set method to `POST`
3. URL: `http://localhost:3000/api/attendance`
4. Headers: `Content-Type: application/json`
5. Body: Raw JSON (see sample above)
6. Click Send

---

## 📈 Rate Limiting

ESP32 sends data **once per minute per student**.

**Expected traffic**:
- 2 students = 2 requests/minute
- 10 students = 10 requests/minute
- 50 students = 50 requests/minute

**Recommended**: No rate limiting needed for typical classroom sizes.

---

## 🔄 Data Flow Diagram

```
┌─────────────┐         ┌─────────────┐         ┌─────────────┐
│   ESP32     │         │  HTTP API   │         │  Database   │
│   Scanner   │         │   Server    │         │             │
└──────┬──────┘         └──────┬──────┘         └──────┬──────┘
       │                       │                       │
       │ POST /api/attendance  │                       │
       ├──────────────────────>│                       │
       │ {studentId, code,...} │                       │
       │                       │                       │
       │                       │ INSERT record         │
       │                       ├──────────────────────>│
       │                       │                       │
       │                       │<──────────────────────┤
       │                       │ Success              │
       │                       │                       │
       │<──────────────────────┤                       │
       │ {success: true}       │                       │
       │                       │                       │
```

---

## 📝 Database Schema Suggestion

### MongoDB
```javascript
{
  _id: ObjectId("..."),
  studentId: 1,
  code: "1P",
  name: "Mathumitha R",
  rollNumber: "310622205081",
  year: 4,
  department: "IT-B",
  location: "CLASSROOM_A",
  status: "Present",
  onDuty: false,
  period: 2,
  date: "02-01-2026",
  time: "10:45 AM",
  timestamp: ISODate("2026-01-02T10:45:00Z")
}
```

### MySQL
```sql
CREATE TABLE attendance (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_id INT NOT NULL,
  code VARCHAR(5) NOT NULL,
  name VARCHAR(100) NOT NULL,
  roll_number VARCHAR(20) NOT NULL,
  year INT NOT NULL,
  department VARCHAR(50) NOT NULL,
  location VARCHAR(50) NOT NULL,
  status ENUM('Present', 'Absent') NOT NULL,
  on_duty BOOLEAN DEFAULT FALSE,
  period INT DEFAULT 0,
  date DATE NOT NULL,
  time VARCHAR(20) NOT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_student (student_id),
  INDEX idx_date (date),
  INDEX idx_code (code)
);
```

---

**Last Updated**: January 2, 2026  
**Version**: 1.0
