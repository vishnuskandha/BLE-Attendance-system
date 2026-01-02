// Vercel Serverless Function - Attendance API
// Deploy to Vercel: https://vercel.com

// In-memory storage (for demo - use database in production)
let attendanceRecords = [];

export default function handler(req, res) {
  // Enable CORS for GitHub Pages
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  
  // Handle preflight
  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }

  // POST - Receive attendance from ESP32
  if (req.method === 'POST') {
    try {
      const record = {
        ...req.body,
        id: Date.now().toString(),
        receivedAt: new Date().toISOString()
      };
      
      // Validate required fields
      if (!record.studentId || !record.code) {
        return res.status(400).json({
          success: false,
          error: 'Missing required fields: studentId, code'
        });
      }
      
      // Check for duplicate in same period
      const existingIndex = attendanceRecords.findIndex(r => 
        r.studentId === record.studentId && 
        r.date === record.date && 
        r.period === record.period
      );
      
      if (existingIndex >= 0) {
        // Update existing record
        attendanceRecords[existingIndex] = record;
      } else {
        // Add new record
        attendanceRecords.push(record);
      }
      
      // Keep only last 1000 records (memory limit)
      if (attendanceRecords.length > 1000) {
        attendanceRecords = attendanceRecords.slice(-1000);
      }
      
      console.log(`✅ Attendance: ${record.name} - ${record.code}`);
      
      return res.status(200).json({
        success: true,
        message: 'Attendance recorded',
        recordId: record.id
      });
      
    } catch (error) {
      console.error('Error:', error);
      return res.status(500).json({
        success: false,
        error: 'Server error'
      });
    }
  }
  
  // GET - Fetch attendance records
  if (req.method === 'GET') {
    try {
      const { date, studentId, irregularities } = req.query;
      
      let filtered = [...attendanceRecords];
      
      // Filter by date
      if (date) {
        filtered = filtered.filter(r => r.date === date);
      }
      
      // Filter by student
      if (studentId) {
        filtered = filtered.filter(r => r.studentId === parseInt(studentId));
      }
      
      // Filter irregularities only
      if (irregularities === 'true') {
        filtered = filtered.filter(r => r.status === 'Absent');
      }
      
      // Sort by date and time (newest first)
      filtered.sort((a, b) => {
        const dateA = new Date(`${a.date} ${a.time}`);
        const dateB = new Date(`${b.date} ${b.time}`);
        return dateB - dateA;
      });
      
      return res.status(200).json(filtered);
      
    } catch (error) {
      console.error('Error:', error);
      return res.status(500).json({
        success: false,
        error: 'Server error'
      });
    }
  }
  
  // Method not allowed
  return res.status(405).json({
    success: false,
    error: 'Method not allowed'
  });
}
