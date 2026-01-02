// Vercel Serverless Function - Students API
// Returns list of registered students

const students = [
  {
    id: 1,
    name: "Mathumitha R",
    rollNumber: "310622205081",
    department: "IT-B",
    year: 4,
    beaconMac: "0E:A5:25:A0:00:16"
  },
  {
    id: 2,
    name: "Lipsa Sahoo",
    rollNumber: "310622205075",
    department: "IT-B",
    year: 4,
    beaconMac: "0E:A5:25:A0:00:13"
  }
];

export default function handler(req, res) {
  // Enable CORS
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  
  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }
  
  if (req.method === 'GET') {
    return res.status(200).json(students);
  }
  
  return res.status(405).json({ error: 'Method not allowed' });
}
