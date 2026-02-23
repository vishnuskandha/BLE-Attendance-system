export default async function handler(req, res) {
    // Return environment variables (safely masked) to check if KV is connected
    res.status(200).json({
        hasKvUrl: !!process.env.KV_URL,
        hasKvRestApiUrl: !!process.env.KV_REST_API_URL,
        hasKvRestApiToken: !!process.env.KV_REST_API_TOKEN,
        nodeEnv: process.env.NODE_ENV,
        region: process.env.VERCEL_REGION || 'unknown',
        kvUrlStart: process.env.KV_URL ? process.env.KV_URL.substring(0, 15) + '...' : null
    });
}
