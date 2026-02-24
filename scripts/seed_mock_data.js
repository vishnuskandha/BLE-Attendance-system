
const { createClient } = require('redis');

// Connection string from api/attendance.js
const REDIS_URL = 'redis://default:yxkNg5aQSuARBBJOxNUpy1upl0bGZchb@redis-17896.c263.us-east-1-2.ec2.cloud.redislabs.com:17896';

const client = createClient({
    url: REDIS_URL
});

client.on('error', (err) => console.log('Redis Client Error', err));

const students = [
    {
        studentId: 1,
        name: "Mathumitha R",
        rollNumber: "310622205081",
        year: 4,
        department: "IT-B"
    },
    {
        studentId: 2,
        name: "Lipsa Sahoo",
        rollNumber: "310622205075",
        year: 4,
        department: "IT-B"
    }
];

async function seed() {
    console.log('Connecting to Redis...');
    await client.connect();
    console.log('Connected!');

    const records = [];
    const now = new Date();

    // Generate records for the last 10 days
    for (let i = 0; i < 10; i++) {
        const date = new Date();
        date.setDate(now.getDate() - i);

        const dateStr = `${String(date.getDate()).padStart(2, '0')}-${String(date.getMonth() + 1).padStart(2, '0')}-${date.getFullYear()}`;

        students.forEach(student => {
            // 3 periods per day
            for (let period = 1; period <= 3; period++) {
                // Randomly make them absent (20% chance)
                const isAbsent = Math.random() < 0.2;
                const status = isAbsent ? 'Absent' : 'Present';

                const rec = {
                    ...student,
                    id: `mock-${dateStr}-${student.studentId}-${period}`,
                    code: `${student.studentId}${status === 'Present' ? 'P' : 'A'}`,
                    location: "CLASSROOM_A",
                    status: status,
                    onDuty: false,
                    period: period,
                    date: dateStr,
                    time: period === 1 ? "09:00 AM" : (period === 2 ? "11:00 AM" : "02:00 PM"),
                    receivedAt: date.toISOString()
                };

                // For Period status consistency in table
                if (period === 1) rec.period1 = status;
                if (period === 2) rec.period2 = status;
                if (period === 3) rec.period3 = status;

                records.push(rec);
            }
        });
    }

    console.log(`Generated ${records.length} mock records.`);

    // Sort by date (descending)
    records.sort((a, b) => new Date(b.receivedAt) - new Date(a.receivedAt));

    await client.set('attendance_records', JSON.stringify(records));
    console.log('✅ Mock data seeded successfully!');

    await client.disconnect();
}

seed().catch(err => {
    console.error('Seeding failed:', err);
    process.exit(1);
});
