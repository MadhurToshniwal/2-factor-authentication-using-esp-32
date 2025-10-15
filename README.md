# 🔐 2-Factor Authentication using ESP32

![ESP32 2FA Band](esp%2032.jpg)

A secure, hardware-based 2FA (Two-Factor Authentication) system using ESP32 microcontroller that provides physical confirmation for sensitive actions through a wearable device.

## 🌟 Features

- **🔒 Hardware-based Security**: Physical button press required for authentication
- **📡 Real-time Communication**: MQTT over TLS for secure messaging
- **🔐 Cryptographic Signing**: HMAC-SHA256 for tamper-proof confirmations
- **💡 Visual Feedback**: LED indicators for confirmation requests
- **🌐 Web Interface**: React-based frontend for device management
- **📱 User-friendly**: Simple registration and confirmation process
- **🔄 Auto-reconnection**: Robust WiFi and MQTT connection handling

## 🏗️ System Architecture

```
┌─────────────────┐    HTTPS/WSS     ┌─────────────────┐    MQTT/TLS    ┌─────────────────┐
│                 │ ◄──────────────► │                 │ ◄────────────► │                 │
│  React Frontend │                  │  Node.js Server │                │   ESP32 Device  │
│   (Port 3001)   │                  │   (Port 3000)   │                │   (2FA Band)    │
│                 │                  │                 │                │                 │
└─────────────────┘                  └─────────────────┘                └─────────────────┘
        │                                      │                                  │
        │                                      │                                  │
        ▼                                      ▼                                  ▼
┌─────────────────┐                  ┌─────────────────┐                ┌─────────────────┐
│ User Interface  │                  │ Device Registry │                │ Physical Button │
│ Authentication  │                  │ MQTT Broker     │                │ Status LED      │
│ Private Data    │                  │ Challenge Gen   │                │ WiFi Module     │
└─────────────────┘                  └─────────────────┘                └─────────────────┘
```

## 🚀 Quick Start

### Prerequisites

- **Hardware**: ESP32 development board
- **Software**: Node.js (v16+), Arduino IDE, Git
- **Services**: WiFi network, MQTT broker access

### 1. Clone the Repository

```bash
git clone https://github.com/MadhurToshniwal/2-factor-authentication-using-esp-32.git
cd 2-factor-authentication-using-esp-32
```

### 2. Backend Setup

```bash
cd backend
npm install
npm start
```
**Backend runs on:** http://localhost:3000

### 3. Frontend Setup

```bash
cd frontend
npm install
npm start
```
**Frontend runs on:** http://localhost:3001

### 4. ESP32 Setup

1. Open `esp32-firmware/secure_2fa_band.ino` in Arduino IDE
2. Install required libraries:
   - WiFi
   - PubSubClient
   - ArduinoJson
   - Preferences
3. Update WiFi credentials in the code
4. Upload to ESP32
5. Open Serial Monitor (115200 baud) to get device pairing info

### 5. Device Registration

1. Open http://localhost:3001
2. Sign up/Login
3. Copy Device ID and Secret from ESP32 Serial Monitor
4. Register device in web interface

## 📋 Project Structure

```
📁 2-factor-authentication-using-esp-32/
├── 📁 backend/                    # Node.js server
│   ├── server.js                  # Main server file
│   ├── package.json              # Dependencies
│   └── .env                      # Environment variables
├── 📁 frontend/                   # React application
│   ├── 📁 src/
│   │   ├── App.js                # Main React component
│   │   ├── components/           # React components
│   │   └── ...
│   ├── package.json              # Dependencies
│   └── .env                      # Environment variables
├── 📁 esp32-firmware/            # ESP32 Arduino code
│   └── secure_2fa_band.ino       # Main firmware file
├── esp 32.jpg                    # Project image
├── README.md                     # This file
└── SETUP-GUIDE.md               # Detailed setup guide
```

## 🔧 How It Works

### 1. **Device Registration**
```
User → ESP32: Upload firmware
ESP32 → User: Display Device ID & Secret
User → WebApp: Enter device credentials
WebApp → Server: Register device
Server → User: Registration confirmed
```

### 2. **2FA Confirmation Flow**
```
User → WebApp: Request private data
WebApp → Server: Generate challenge
Server → ESP32: Send challenge via MQTT
ESP32 → ESP32: LED turns ON
User → ESP32: Press button
ESP32 → ESP32: Sign challenge with HMAC-SHA256
ESP32 → Server: Send signed response
Server → WebApp: Verify & grant access
WebApp → User: Display private data
```

## 🛠️ Technical Specifications

### Hardware Requirements
- **ESP32 Development Board** (any variant)
- **Push Button** (connected to GPIO 0)
- **LED** (built-in on GPIO 2)
- **WiFi Network** (2.4GHz)

### Software Stack
- **Frontend**: React.js, Clerk Authentication, WebSocket
- **Backend**: Node.js, Express.js, MQTT Client, Crypto
- **Firmware**: Arduino C++, ESP32 Libraries
- **Communication**: MQTT over TLS, HTTPS, WebSocket

### Security Features
- **HMAC-SHA256**: Cryptographic message authentication
- **Hardware RNG**: ESP32's true random number generator
- **TLS Encryption**: Secure MQTT communication
- **Challenge-Response**: Prevents replay attacks
- **Physical Confirmation**: Requires button press

## 🔒 Security Model

### Device Secret Generation
```cpp
// 32-byte cryptographically secure random key
generateSecureRandom(secretKey, 32);
String secretHex = hexEncode(secretKey, 32); // 64 hex characters
```

### Challenge Signing
```cpp
// HMAC-SHA256 signature generation
uint8_t signature[32];
computeHMAC(challenge, secretKey, secretLen, signature);
String signatureHex = hexEncode(signature, 32);
```

### Verification Process
```javascript
// Server-side signature verification
const expectedSignature = crypto
  .createHmac('sha256', Buffer.from(deviceSecret, 'hex'))
  .update(challenge)
  .digest('hex');
```

## 📡 MQTT Communication

### Topics Structure
```
devices/{deviceId}/challenge    # Server → ESP32 (challenges)
devices/{deviceId}/response     # ESP32 → Server (confirmations)
devices/{deviceId}/status       # ESP32 → Server (online/offline)
```

### Message Format
```json
// Challenge Message
{
  "challenge": "random_challenge_string",
  "confirmationId": "unique_confirmation_id",
  "action": "Access Private Information"
}

// Response Message
{
  "signatureHex": "hmac_sha256_signature",
  "confirmationId": "matching_confirmation_id",
  "timestamp": 1234567890
}
```

## 🚨 Troubleshooting

### Common Issues

1. **"Invalid device secret format"**
   - Ensure ESP32 firmware is latest version
   - Check Serial Monitor for 64-character hex secret
   - Re-upload firmware if needed

2. **WiFi Connection Failed**
   - Verify SSID and password in firmware
   - Check 2.4GHz network availability
   - Move ESP32 closer to router

3. **MQTT Connection Issues**
   - Check internet connectivity
   - Verify broker.emqx.io accessibility
   - Check firewall settings

4. **Registration Failed**
   - Ensure backend server is running on port 3000
   - Check device ID format (MAC address)
   - Verify 64-character device secret

### Debug Commands

```bash
# Check running processes
netstat -ano | findstr ":3000|:3001"

# Kill conflicting processes
taskkill /PID <process_id> /F

# Check ESP32 connection
# Open Arduino IDE Serial Monitor at 115200 baud
```

## 🎯 Use Cases

- **🏦 Banking Applications**: Secure transaction confirmations
- **🏢 Corporate Security**: Server access authorization  
- **🏠 Smart Home**: Critical device control confirmation
- **💻 Development**: Secure API access management
- **🔐 Password Managers**: Master password confirmation

## 🛣️ Roadmap

- [ ] **Mobile App**: iOS/Android companion app
- [ ] **Multi-Device**: Support for multiple ESP32 devices
- [ ] **Biometric**: Fingerprint sensor integration
- [ ] **Battery**: Rechargeable battery support
- [ ] **Wearable**: Custom PCB for wrist-worn form factor
- [ ] **Cloud**: AWS/Azure cloud deployment
- [ ] **Analytics**: Usage statistics and security insights

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md).

### Development Setup
1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Madhur Toshniwal**
- Email: madhurtoshniwal03@gmail.com
- GitHub: [@MadhurToshniwal](https://github.com/MadhurToshniwal)
- Project: [2FA using ESP32](https://github.com/MadhurToshniwal/2-factor-authentication-using-esp-32)

## 🙏 Acknowledgments

- ESP32 Community for excellent documentation
- EMQX for providing free MQTT broker
- React and Node.js communities
- Arduino IDE developers

## 📞 Support

If you encounter any issues or have questions:

1. Check the [Troubleshooting](#-troubleshooting) section
2. Search existing [GitHub Issues](https://github.com/MadhurToshniwal/2-factor-authentication-using-esp-32/issues)
3. Create a new issue with detailed description
4. Email: madhurtoshniwal03@gmail.com

---

⭐ **Star this repository if you found it helpful!**

![Built with Love](https://img.shields.io/badge/Built%20with-❤️-red)
![ESP32](https://img.shields.io/badge/ESP32-Hardware-blue)
![React](https://img.shields.io/badge/React-Frontend-61DAFB)
![Node.js](https://img.shields.io/badge/Node.js-Backend-339933)
![MQTT](https://img.shields.io/badge/MQTT-Communication-FF6600)
![Security](https://img.shields.io/badge/Security-HMAC--SHA256-red)