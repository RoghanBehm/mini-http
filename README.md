A minimal, handwritten HTTP/1.1 server.

### Current Features
- HTTP/1.1 support with persistent connections
- Basic routing (`/echo`, `/user-agent`, `/files`)
- File serving and POST file upload
- Optional gzip compression (Accept-Encoding: gzip)
- Concurrency handled via threadpool

| Header | Purpose |
|--------|---------|
| `Host` | Required by HTTP/1.1; used for request validation (not enforced here) |
| `User-Agent` | Reflected in `/user-agent` route |
| `Accept-Encoding` | Supports `gzip` (if present); otherwise falls back to plain text |
| `Connection` | Supports `close`, connections stay open by default |
| `Content-Length` | Used to determine POST request body size |
| `Content-Type` | Validates uploaded data in POST requests (`application/octet-stream`) |
