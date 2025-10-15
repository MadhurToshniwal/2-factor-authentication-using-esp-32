import express from 'express';
import { clerkClient, requireAuth } from '@clerk/clerk-sdk-node';
import mqtt from 'mqtt';
import crypto from 'crypto';
import cors from 'cors';
import dotenv from 'dotenv';
import { WebSocketServer } from 'ws';
import { createServer } from 'http';

dotenv.config();

const app = express();
const server = createServer(app);

// WebSocket setup for real-time updates
const wss = new WebSocketServer({ server });
const wsClients = new Map(); // Map<userId, WebSocket>

// Middleware
app.use(express.json());
app.use(cors({
  origin: ['http://localhost:3001', 'http://localhost:3000'],
  credentials: true
}));

// Clerk middleware for authentication
const authenticateUser = async (req, res, next) => {
  try {
    const { authorization } = req.headers;
    if (!authorization) {
      return res.status(401).json({ error: 'No authorization header' });
    }

    const token = authorization.replace('Bearer ', '');
    const user = await clerkClient.verifyToken(token);
    
    if (!user) {
      return res.status(401).json({ error: 'Invalid token' });
    }

    req.auth = { userId: user.sub };
    next();
  } catch (error) {
    console.error('Authentication error:', error);
    res.status(401).json({ error: 'Authentication failed' });
  }
};

// In-memory store for devices (replace with database in production)
// Structure: { deviceId: { userId, secret (hex), lastChallenge, deviceName } }
const devices = new Map();

// Store for pending confirmations
// Structure: { confirmationId: { userId, deviceId, action, timestamp, status } }
const pendingConfirmations = new Map();

// MQTT Configuration
const mqttOptions = {
  // For public EMQX broker - no auth needed
  // For private brokers, add username/password
  rejectUnauthorized: false, // For demo purposes, set to true in production
};

console.log('Connecting to MQTT broker:', process.env.MQTT_BROKER);
const mqttClient = mqtt.connect(process.env.MQTT_BROKER, mqttOptions);

mqttClient.on('connect', () => {
  console.log('✅ MQTT connected successfully');
});

mqttClient.on('error', (error) => {
  console.error('❌ MQTT connection error:', error);
});

mqttClient.on('offline', () => {
  console.log('⚠️  MQTT offline');
});

// WebSocket connection handling
wss.on('connection', (ws, request) => {
  console.log('New WebSocket connection');
  
  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message.toString());
      if (data.type === 'auth' && data.userId) {
        wsClients.set(data.userId, ws);
        ws.send(JSON.stringify({ type: 'auth_success' }));
        console.log(`User ${data.userId} connected via WebSocket`);
      }
    } catch (error) {
      console.error('WebSocket message error:', error);
    }
  });
  
  ws.on('close', () => {
    // Remove from clients map
    for (const [userId, client] of wsClients.entries()) {
      if (client === ws) {
        wsClients.delete(userId);
        console.log(`User ${userId} disconnected from WebSocket`);
        break;
      }
    }
  });
});

// Broadcast to specific user
function notifyUser(userId, message) {
  const client = wsClients.get(userId);
  if (client && client.readyState === client.OPEN) {
    client.send(JSON.stringify(message));
  }
}

// API Routes

// Health check
app.get('/api/health', (req, res) => {
  res.json({ 
    status: 'ok', 
    mqtt: mqttClient.connected ? 'connected' : 'disconnected',
    timestamp: new Date().toISOString()
  });
});

// Get user ID for WebSocket authentication
app.get('/api/user-id', authenticateUser, (req, res) => {
  res.json({ userId: req.auth.userId });
});

// Get user's registered devices
app.get('/api/devices', authenticateUser, (req, res) => {
  const userDevices = [];
  for (const [deviceId, device] of devices.entries()) {
    if (device.userId === req.auth.userId) {
      userDevices.push({
        deviceId,
        deviceName: device.deviceName || 'Unnamed Device',
        registeredAt: device.registeredAt,
        lastSeen: device.lastSeen
      });
    }
  }

  res.json({ devices: userDevices });
});

// Register a new device
app.post('/api/register-device', authenticateUser, (req, res) => {
  const { deviceId, deviceSecret, deviceName } = req.body;
  
  if (!deviceId || !deviceSecret) {
    return res.status(400).json({ error: 'Missing deviceId or deviceSecret' });
  }

  // Validate deviceSecret is hex and proper length (64 chars = 32 bytes)
  if (!/^[0-9a-fA-F]{64}$/.test(deviceSecret)) {
    return res.status(400).json({ error: 'Invalid device secret format' });
  }

  // Check if device already exists
  if (devices.has(deviceId)) {
    return res.status(409).json({ error: 'Device already registered' });
  }

  // Register the device
  devices.set(deviceId, {
    userId: req.auth.userId,
    secret: deviceSecret.toLowerCase(),
    deviceName: deviceName || `Device ${deviceId.slice(-6)}`,
    registeredAt: new Date().toISOString(),
    lastSeen: new Date().toISOString()
  });

  // Subscribe to device responses
  mqttClient.subscribe(`devices/${deviceId}/response`, (err) => {
    if (err) {
      console.error('Failed to subscribe to device responses:', err);
    } else {
      console.log(`Subscribed to devices/${deviceId}/response`);
    }
  });

  console.log(`✅ Device registered: ${deviceId} for user: ${req.auth.userId}`);
  
  res.json({ 
    success: true, 
    message: 'Device registered successfully',
    deviceId,
    deviceName: devices.get(deviceId).deviceName
  });
});

// Request confirmation from device
app.post('/api/request-confirm', authenticateUser, (req, res) => {
  const { deviceId, action } = req.body;
  
  if (!deviceId) {
    return res.status(400).json({ error: 'Missing deviceId' });
  }

  const device = devices.get(deviceId);
  if (!device || device.userId !== req.auth.userId) {
    return res.status(404).json({ error: 'Device not found or not authorized' });
  }

  // Generate challenge
  const challenge = crypto.randomBytes(32).toString('hex');
  const confirmationId = crypto.randomUUID();
  const timestamp = Date.now();

  // Store pending confirmation
  pendingConfirmations.set(confirmationId, {
    userId: req.auth.userId,
    deviceId,
    action: action || 'Generic confirmation',
    timestamp,
    status: 'pending',
    challenge
  });

  // Update device last challenge
  device.lastChallenge = {
    challenge,
    timestamp,
    confirmationId
  };

  // Publish challenge to device via MQTT
  const challengePayload = {
    challenge,
    confirmationId,
    action: action || 'Confirm action',
    timestamp
  };

  mqttClient.publish(`devices/${deviceId}/challenge`, JSON.stringify(challengePayload), (err) => {
    if (err) {
      console.error('Failed to publish challenge:', err);
      return res.status(500).json({ error: 'Failed to send challenge to device' });
    }
    
    console.log(`📤 Challenge sent to device ${deviceId}`);
    
    // Set timeout for challenge (5 minutes)
    setTimeout(() => {
      const confirmation = pendingConfirmations.get(confirmationId);
      if (confirmation && confirmation.status === 'pending') {
        confirmation.status = 'expired';
        notifyUser(req.auth.userId, {
          type: 'confirmation_expired',
          confirmationId,
          action: confirmation.action
        });
      }
    }, 5 * 60 * 1000);
    
    res.json({ 
      success: true, 
      confirmationId,
      message: 'Challenge sent to device. Please press the button on your device to confirm.'
    });
  });
});

// Get confirmation status
app.get('/api/confirmation/:confirmationId', authenticateUser, (req, res) => {
  const { confirmationId } = req.params;
  const confirmation = pendingConfirmations.get(confirmationId);

  if (!confirmation || confirmation.userId !== req.auth.userId) {
    return res.status(404).json({ error: 'Confirmation not found' });
  }

  res.json({
    confirmationId,
    status: confirmation.status,
    action: confirmation.action,
    timestamp: confirmation.timestamp
  });
});

// Get user's confirmation history
app.get('/api/confirmations', authenticateUser, (req, res) => {
  const userConfirmations = [];
  for (const [confirmationId, confirmation] of pendingConfirmations.entries()) {
    if (confirmation.userId === req.auth.userId) {
      userConfirmations.push({
        confirmationId,
        action: confirmation.action,
        status: confirmation.status,
        timestamp: confirmation.timestamp,
        deviceId: confirmation.deviceId
      });
    }
  }

  // Sort by timestamp (newest first)
  userConfirmations.sort((a, b) => b.timestamp - a.timestamp);

  res.json({ confirmations: userConfirmations });
});

// Remove device
app.delete('/api/devices/:deviceId', authenticateUser, (req, res) => {
  const { deviceId } = req.params;
  const device = devices.get(deviceId);

  if (!device || device.userId !== req.auth.userId) {
    return res.status(404).json({ error: 'Device not found or not authorized' });
  }

  // Unsubscribe from MQTT topics
  mqttClient.unsubscribe(`devices/${deviceId}/response`);
  
  // Remove device
  devices.delete(deviceId);

  console.log(`🗑️  Device removed: ${deviceId} for user: ${req.auth.userId}`);
  
  res.json({ 
    success: true, 
    message: 'Device removed successfully' 
  });
});

// Handle device responses via MQTT
mqttClient.on('message', (topic, payload) => {
  try {
    const parts = topic.split('/');
    if (parts.length === 3 && parts[0] === 'devices' && parts[2] === 'response') {
      const deviceId = parts[1];
      const device = devices.get(deviceId);
      
      if (!device) {
        console.log(`❌ Response from unknown device: ${deviceId}`);
        return;
      }

      const response = JSON.parse(payload.toString());
      const { signatureHex, confirmationId } = response;

      if (!signatureHex || !confirmationId) {
        console.log(`❌ Invalid response format from device ${deviceId}`);
        return;
      }

      const confirmation = pendingConfirmations.get(confirmationId);
      if (!confirmation) {
        console.log(`❌ Unknown confirmation ID: ${confirmationId}`);
        return;
      }

      if (confirmation.status !== 'pending') {
        console.log(`❌ Confirmation ${confirmationId} is not pending (status: ${confirmation.status})`);
        return;
      }

      // Verify HMAC signature
      const expectedSignature = crypto
        .createHmac('sha256', Buffer.from(device.secret, 'hex'))
        .update(confirmation.challenge)
        .digest('hex');

      const providedSignature = signatureHex.toLowerCase();

      if (crypto.timingSafeEqual(Buffer.from(expectedSignature, 'hex'), Buffer.from(providedSignature, 'hex'))) {
        // Valid signature - mark confirmation as confirmed
        confirmation.status = 'confirmed';
        confirmation.confirmedAt = Date.now();

        // Update device last seen
        device.lastSeen = new Date().toISOString();

        console.log(`✅ Device confirmed! User: ${device.userId}, Action: ${confirmation.action}`);

        // Notify user via WebSocket
        notifyUser(device.userId, {
          type: 'confirmation_success',
          confirmationId,
          action: confirmation.action,
          deviceId
        });

      } else {
        // Invalid signature
        confirmation.status = 'failed';
        console.log(`❌ Invalid signature from device ${deviceId}`);
        
        // Notify user via WebSocket
        notifyUser(device.userId, {
          type: 'confirmation_failed',
          confirmationId,
          action: confirmation.action,
          error: 'Invalid signature'
        });
      }
    }
  } catch (error) {
    console.error('Error processing MQTT message:', error);
  }
});

// Error handling middleware
app.use((error, req, res, next) => {
  console.error('Server error:', error);
  res.status(500).json({ error: 'Internal server error' });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`🚀 Server running on port ${PORT}`);
  console.log(`📱 Frontend should connect to: http://localhost:${PORT}`);
});

// Graceful shutdown
process.on('SIGTERM', () => {
  console.log('Shutting down gracefully...');
  mqttClient.end();
  server.close(() => {
    process.exit(0);
  });
});
