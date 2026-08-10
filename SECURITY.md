# Security Policy

## Reporting a Vulnerability

Do not open a public issue for security problems. Instead, email the maintainer
directly or open a private advisory via GitHub's "Report a vulnerability" flow on
the repository page. Include:

- A description of the vulnerability and the affected component.
- Steps to reproduce.
- The impact you observed.

You will receive an acknowledgement within 5 business days and a follow-up once the
issue is triaged.

## Security Notes

- **ESP32 WiFi credentials** are hardcoded at the top of
  `esp32_attendance_optimized.ino`. Treat that sketch as untrusted to commit: keep
  real SSIDs and passwords out of the repository and configure them only in local
  copies before flashing. The repository only ever contains placeholders.
- **Dashboard login** is a hardcoded demo credential in `index.html`
  (`Principal` / `admin`). It is not a real authentication layer - change it (or move
  to a proper identity provider) before any public deployment.
- **Redis connection string** is read from the `REDIS_URL` / `KV_URL` environment
  variables at runtime. Keep it in Vercel secret environment variables, never in
  source code or client-side files.
- **Roboflow publishable key** in `index.html` is a client-side public key by design
  (Roboflow publishable keys are meant to be embedded in browsers). It cannot replace
  server-side authentication for the API.
- **API protection** - the `/api/*` endpoints currently rely on CORS and should not
  be treated as a security boundary. For production, add API-key or token
  authentication (see [DEPLOYMENT.md](DEPLOYMENT.md)) and rate limiting.
- If you believe a real secret was ever committed, rotate it immediately and report
  it via the process above.
