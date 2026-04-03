
const { createClient } = require('redis');

const REDIS_URL = process.env.REDIS_URL || process.env.KV_URL;

if (!REDIS_URL) {
    console.error('Missing REDIS_URL/KV_URL environment variable');
    process.exit(1);
}

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
