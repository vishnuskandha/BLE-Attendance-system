export default async function handler(req, res) {
    if (process.env.NODE_ENV === 'production') {
        return res.status(404).json({ error: 'Not found' });
    }

    // Return environment variables (safely masked) to check if KV is connected
    res.status(200).json({
        hasKvUrl: !!process.env.KV_URL,
        hasKvRestApiUrl: !!process.env.KV_REST_API_URL,
        hasKvRestApiToken: !!process.env.KV_REST_API_TOKEN,
        nodeEnv: process.env.NODE_ENV,
        region: process.env.VERCEL_REGION || 'unknown'
    });
}
