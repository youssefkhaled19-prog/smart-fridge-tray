const express = require('express');
const http = require('http');
const cors = require('cors');
const ws = require('ws');

const HTTP_PORT = 3000;

let latestData = {
  count: 0,
  weight_g: 0,
  low_stock: false,
  threshold: 3,
  timestamp: null,
  connected: false,
};

const app = express();
app.use(cors());
app.use(express.json());
app.use(express.static('C:\\Users\\admin\\Desktop\\fridgewatch\\webapp'));

app.get('/api/status', (req, res) => res.json(latestData));

app.post('/api/data', (req, res) => {
  const data = req.body;
  latestData = { ...latestData, ...data, timestamp: new Date().toISOString(), connected: true };
  console.log('[Data] count=' + latestData.count + ' | weight=' + latestData.weight_g + 'g | low=' + latestData.low_stock);
  broadcast({ type: 'update', data: latestData });
  res.json({ success: true });
});

app.post('/api/threshold', (req, res) => {
  const { value } = req.body;
  if (typeof value !== 'number' || value < 1) return res.status(400).json({ error: 'Invalid' });
  latestData.threshold = value;
  console.log('[Config] Threshold: ' + value);
  res.json({ success: true, threshold: value });
});

app.post('/api/tare', (req, res) => {
  latestData.count = 0;
  latestData.weight_g = 0;
  broadcast({ type: 'update', data: latestData });
  res.json({ success: true });
});

const httpServer = http.createServer(app);
const wss = new ws.WebSocketServer({ server: httpServer });
const clients = new Set();

wss.on('connection', (socket) => {
  clients.add(socket);
  console.log('[WS] Browser connected. Total: ' + clients.size);
  socket.send(JSON.stringify({ type: 'update', data: latestData }));
  socket.on('close', () => { clients.delete(socket); console.log('[WS] Browser disconnected. Total: ' + clients.size); });
  socket.on('error', () => clients.delete(socket));
});

function broadcast(message) {
  const json = JSON.stringify(message);
  for (const client of clients) {
    if (client.readyState === ws.WebSocket.OPEN) client.send(json);
  }
}

httpServer.listen(HTTP_PORT, () => {
  console.log('[HTTP] Server running at http://localhost:' + HTTP_PORT);
  console.log('[WS]   WebSocket ready');
  console.log('Open your browser at: http://localhost:3000');
});
