import { kv } from '@vercel/kv';

export default async function handler(req, res) {
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

      // Reject records outside class hours (period 0 = no active period)
      if (!record.period || record.period === 0) {
        return res.status(200).json({
          success: true,
          message: 'Skipped - outside class hours',
          recordId: null
        });
      }

      // Fetch existing records from KV
      let attendanceRecords = await kv.get('attendance_records') || [];

      // Check for duplicate in same period
      const existingIndex = attendanceRecords.findIndex(r =>
        r.studentId === record.studentId &&
        r.date === record.date &&
        r.period === record.period
      );

      if (existingIndex >= 0) {
        const existingRecord = attendanceRecords[existingIndex];

        // "Once Present, Always Present" logic:
        if (existingRecord.status === 'Present' && record.status === 'Absent') {
          return res.status(200).json({
            success: true,
            message: 'Ignored - student already marked Present for this period',
            recordId: existingRecord.id
          });
        }

        // Update existing record
        attendanceRecords[existingIndex] = record;
      } else {
        // Add new record
        attendanceRecords.push(record);
      }

      // Keep only last 2000 records for KV efficiency
      if (attendanceRecords.length > 2000) {
        attendanceRecords = attendanceRecords.slice(-2000);
      }

      // Save back to KV
      await kv.set('attendance_records', attendanceRecords);

      console.log(`✅ Attendance (KV): ${record.name} - ${record.code}`);

      return res.status(200).json({
        success: true,
        message: 'Attendance recorded in database',
        recordId: record.id
      });

    } catch (error) {
      console.error('KV Error (POST):', error);
      return res.status(500).json({
        success: false,
        error: 'Database error'
      });
    }
  }

  // GET - Fetch attendance records from KV
  if (req.method === 'GET') {
    try {
      const { date, studentId, irregularities } = req.query;

      // Fetch all records from KV
      let records = await kv.get('attendance_records') || [];
      let filtered = [...records];

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
      console.error('KV Error (GET):', error);
      return res.status(500).json({
        success: false,
        error: 'Database error'
      });
    }
  }

  // Method not allowed
  return res.status(405).json({
    success: false,
    error: 'Method not allowed'
  });
}
