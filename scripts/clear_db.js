
const { createClient } = require('redis');

// Connection string from api/attendance.js
const REDIS_URL = 'redis://default:yxkNg5aQSuARBBJOxNUpy1upl0bGZchb@redis-17896.c263.us-east-1-2.ec2.cloud.redislabs.com:17896';

const client = createClient({
    url: REDIS_URL
});

client.on('error', (err) => console.log('Redis Client Error', err));

async function clearDatabase() {
    console.log('Connecting to Redis...');
    await client.connect();
    console.log('Connected!');

    // The attendance data is stored in the 'attendance_records' key
    console.log('Clearing attendance_records...');
    await client.del('attendance_records');

    // Also clear irregularities memory if it exists
    await client.del('presence_memory');

    console.log('✅ Database cleared successfully!');

    await client.disconnect();
}

clearDatabase().catch(err => {
    console.error('Clear failed:', err);
    process.exit(1);
});
