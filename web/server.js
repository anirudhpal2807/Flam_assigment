// ...existing code...
const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');

function boot(port){
  const app = express();
  app.use(express.static(path.join(__dirname, 'public')));
  app.get('/debug/ping', (req, res) => res.send('pong'));

  // debug endpoint: broadcast small test image (base64 jpeg)
  const testBase64 = '/9j/4AAQSkZJRgABAQAAAQABAAD/2wCEAAkGBxISEhUTExIVFhUVFhUVFRUVFRUVFRUVFRUWFhUVFRUYHSggGBolHRUVITEhJSkrLi4uFx8zODMtNygtLisBCgoKDg0OGhAQGi0lHyUtLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLf/AABEIAKABJAMBIgACEQEDEQH/xAAbAAEAAgMBAQAAAAAAAAAAAAAABQYBAwIBB//EADoQAAIBAgQEAwYEBwAAAAAAAAECAwQRAAUSITEGQVEiYXGBkRQyUrHB0fAjQmJygvEkM0Nz/8QAGQEBAQEBAQEAAAAAAAAAAAAAAAECAwQF/8QAJBEBAAICAgICAwEAAAAAAAAAAAECERIDITFBUXESIjJRcYH/2gAMAwEAAhEDEQA/AO4gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP/2Q==';

  const server = http.createServer(app);
  const wss = new WebSocket.Server({ server, path: '/ws' });

  // keep last frame in memory for HTTP polling fallback
  let lastFrameBase64 = null;

  wss.on('connection', (ws, req) => {
    console.log('ws client connected:', req.socket.remoteAddress);
    ws.on('message', (message) => {
      try { console.log(`ws message received, bytes=${message.length}`); } catch { console.log('ws message received'); }
      wss.clients.forEach(client => {
        if (client !== ws && client.readyState === WebSocket.OPEN) client.send(message);
      });
    });
    ws.on('close', () => console.log('ws client disconnected'));
    ws.on('error', (e) => console.error('ws error', e));
  });

  app.get('/debug/sendTest', (req, res) => {
    lastFrameBase64 = testBase64;
    wss.clients.forEach(client => { if (client.readyState === WebSocket.OPEN) client.send(testBase64); });
    res.send('sent');
  });

  // Simple HTTP endpoint to receive a JPEG frame (base64 or binary) from Android and broadcast to WS clients
  app.post('/ingest', express.raw({ type: ['image/jpeg', 'application/octet-stream'], limit: '10mb' }), (req, res) => {
    let payload;
    if (typeof req.body === 'string') {
      // If someone posts base64 string
      payload = req.body.replace(/^data:image\/jpeg;base64,/, '');
    } else {
      // Binary JPEG -> base64
      payload = Buffer.from(req.body).toString('base64');
    }
    lastFrameBase64 = payload;
    console.log(`ingest: ${Math.round(payload.length/1024)} KB`);
    wss.clients.forEach(client => { if (client.readyState === WebSocket.OPEN) client.send(payload); });
    res.sendStatus(200);
  });

  // HTTP polling fallback to fetch the latest frame
  app.get('/latest', (req, res) => {
    if (!lastFrameBase64) { res.status(204).end(); return; }
    res.setHeader('Content-Type', 'text/plain');
    res.setHeader('Cache-Control', 'no-store');
    res.send(lastFrameBase64);
  });

  let listened = false;
  server.once('error', (err) => {
    if (err && err.code === 'EADDRINUSE') {
      // If user forced a port via env, don't auto-pick silently.
      if (process.env.PORT) {
        console.error(`Port ${port} in use. Set PORT to a different value.`);
        process.exit(1);
      }
      // Ask OS for a free port.
      try { server.close(); } catch {}
      try { wss.close(); } catch {}
      console.warn(`Port ${port} in use, selecting a free port...`);
      boot(0);
    } else {
      console.error('Server error:', err);
      process.exit(1);
    }
  });
  server.once('listening', () => {
    listened = true;
    const addr = server.address();
    const actual = typeof addr === 'object' && addr ? addr.port : port;
    console.log(`Server + WS listening on http://localhost:${actual}`);
  });
  wss.on('error', (e) => console.error('WS error:', e));
  server.listen(port);
}

const START_PORT = (process.env.PORT ? Number(process.env.PORT) : 5173);
boot(START_PORT);