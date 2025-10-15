# Complete Setup Guide - Secure 2FA Band Project

## 📋 Step-by-Step Setup Instructions

### Phase 1: Create Clerk Account (Authentication Service)

#### 1.1 Sign up for Clerk
1. **Visit Clerk**: Go to [https://clerk.com](https://clerk.com)
2. **Click "Get started for free"**
3. **Sign up** with your email or GitHub account
4. **Verify your email** if required

#### 1.2 Create Your Application
1. **Click "Add application"** on the dashboard
2. **Choose application type**:
   - Select "React" (since we're using React frontend)
   - Or select "JavaScript" if React isn't available
3. **Name your application**: "Secure 2FA Band" (or any name you prefer)
4. **Choose authentication methods**:
   - ✅ **Email** (recommended)
   - ✅ **Password** (recommended)  
   - ✅ **Google** (optional, for easier login)
   - ❌ Skip phone/SMS for now (optional)
5. **Click "Create Application"**

#### 1.3 Get Your API Keys
1. **Go to "API Keys"** in the left sidebar
2. **Copy these keys** (you'll need them later):
   - **Publishable Key**: Starts with `pk_test_` or `pk_live_`
   - **Secret Key**: Starts with `sk_test_` or `sk_live_`

#### 1.4 Configure Application Settings (Optional but Recommended)
1. **Go to "User & Authentication"** → **"Email, Phone, Username"**
2. **Set up sign-in methods**:
   - Email address: Required
   - Password: Required
3. **Go to "User & Authentication"** → **"Social Connections"** (if you want social login)
   - Enable Google, GitHub, etc. as desired

---

### Phase 2: Backend Server Setup

#### 2.1 Navigate to Backend Directory
```powershell
cd "d:\IOT project\backend"
```

#### 2.2 Install Dependencies
```powershell
npm install
```

Wait for installation to complete. You should see something like:
```
added 234 packages, and audited 235 packages in 15s
found 0 vulnerabilities
```

#### 2.3 Configure Environment Variables
1. **Open `.env` file** in the backend folder
2. **Replace the placeholder values**:

**Before:**
```env
CLERK_PUBLISHABLE_KEY=your_clerk_publishable_key_here
CLERK_SECRET_KEY=your_clerk_secret_key_here
```

**After (example):**
```env
CLERK_PUBLISHABLE_KEY=pk_test_bG1hc3RlcmtleXBsYWNlaG9sZGVyMTIzNDU2Nzg5MA==
CLERK_SECRET_KEY=sk_test_bWFzdGVyc2VjcmV0a2V5cGxhY2Vob2xkZXIxMjM0NTY3ODkw
```

#### 2.4 Start the Backend Server
```powershell
npm start
```

**Expected Output:**
```
🔄 Connecting to MQTT broker: mqtts://broker.emqx.io:8883
✅ MQTT connected successfully
🚀 Server running on port 3000
📱 Frontend should connect to: http://localhost:3000
```

**If you see errors:**
- ❌ **Port 3000 already in use**: Stop other services or change PORT in .env
- ❌ **MQTT connection failed**: Check internet connection
- ❌ **Clerk key errors**: Verify your API keys are correct

---

### Phase 3: Frontend Web App Setup

#### 3.1 Open New Terminal Window
Keep the backend running and open a new terminal:

```powershell
cd "d:\IOT project\frontend"
```

#### 3.2 Install Dependencies
```powershell
npm install
```

#### 3.3 Configure Environment Variables
1. **Open `.env` file** in the frontend folder
2. **Add your Clerk publishable key**:

**Before:**
```env
REACT_APP_CLERK_PUBLISHABLE_KEY=your_clerk_publishable_key_here
```

**After:**
```env
REACT_APP_CLERK_PUBLISHABLE_KEY=pk_test_bG1hc3RlcmtleXBsYWNlaG9sZGVyMTIzNDU2Nzg5MA==
```

#### 3.4 Start the Frontend
```powershell
npm start
```

**Expected Output:**
```
Compiled successfully!

You can now view secure-2fa-band-frontend in the browser.

  Local:            http://localhost:3001
  On Your Network:  http://192.168.1.100:3001
```

**Your browser should automatically open** to `http://localhost:3001`

---

### Phase 4: ESP32 Hardware Setup

#### 4.1 Hardware Assembly
Connect components to your ESP32:

```
ESP32 Pin  | Component           | Notes
-----------|--------------------|-----------------------
GPIO 0     | Push Button        | Other end to GND
GPIO 2     | LED + 220Ω Resistor| Resistor to GND
3.3V       | Power (if needed)   | 
GND        | Common Ground       | All grounds connected
```

**Breadboard Layout:**
```
ESP32      [Button]     [LED+Resistor]
  |           |              |
GPIO0 --------+              |
GPIO2 -----------------------+
GND   --------+--------------+
```

#### 4.2 Software Preparation
1. **Install Arduino IDE**: Download from [arduino.cc](https://www.arduino.cc/en/software)
2. **Add ESP32 Board Support**:
   - File → Preferences
   - Additional Boards Manager URLs: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "ESP32" → Install
3. **Install Required Libraries**:
   - Tools → Manage Libraries → Search and install:
   - ✅ **ArduinoJson** by Benoit Blanchon
   - ✅ **PubSubClient** by Nick O'Leary

#### 4.3 Configure WiFi Credentials
1. **Open `secure_2fa_band.ino`** in Arduino IDE
2. **Find these lines** (around line 30):
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```
3. **Replace with your actual WiFi**:
```cpp
const char* ssid = "MyHomeWiFi";
const char* password = "MyWiFiPassword123";
```

#### 4.4 Upload Firmware
1. **Connect ESP32** to computer via USB
2. **Select Board**: Tools → Board → ESP32 Dev Module
3. **Select Port**: Tools → Port → (choose your ESP32 port)
4. **Upload**: Click the upload button (→)

**Expected Upload Output:**
```
Sketch uses 894,466 bytes (68%) of program storage space
Hard resetting via RTS pin...
```

#### 4.5 Monitor Serial Output
1. **Open Serial Monitor**: Tools → Serial Monitor
2. **Set baud rate**: 115200
3. **Press ESP32 reset button**

**Expected Serial Output:**
```
🔐 Secure 2FA Confirmation Band
==================================
🔧 Hardware initialized
🆔 Device ID: AA:BB:CC:DD:EE:FF
🔐 Generated and saved new device secret

📋 DEVICE PAIRING INFORMATION
=====================================
Device ID: AA:BB:CC:DD:EE:FF
Device Secret (hex): 1a2b3c4d5e6f7890abcdef1234567890abcdef1234567890abcdef1234567890
Device Name: ESP32 2FA Band
=====================================
📱 Copy the above information to register this device in the web app
🔗 Make sure you're logged into the web app first!

🔄 Connecting to WiFi: MyHomeWiFi
✅ WiFi connected!
📍 IP address: 192.168.1.150
🔄 Connecting to MQTT broker: broker.emqx.io
✅ MQTT connected!
📡 Subscribed to challenge topic: devices/AA:BB:CC:DD:EE:FF/challenge
🚀 Device ready for 2FA confirmations!
```

**⚠️ IMPORTANT: Copy the Device ID and Device Secret** - you'll need these for registration!

---

### Phase 5: Device Registration & Testing

#### 5.1 Access Web Application
1. **Go to**: `http://localhost:3001`
2. **Click "Sign In / Sign Up"**
3. **Create account or sign in** using Clerk

#### 5.2 Register Your ESP32 Device
1. **Click "My Devices" tab**
2. **Click "Add Device"**
3. **Fill in the form**:
   - **Device ID**: `AA:BB:CC:DD:EE:FF` (from serial monitor)
   - **Device Secret**: `1a2b3c4d5e6f...` (64-char hex from serial monitor)
   - **Device Name**: "My Test Band" (optional)
4. **Click "Register Device"**

**Success message**: "Device 'My Test Band' registered successfully!"

#### 5.3 Test the 2FA System
1. **Go to "Demo & Test" tab**
2. **Select your device** from dropdown
3. **Enter action**: "Test Login Confirmation"
4. **Click "🔒 Request Confirmation"**

**What should happen:**
1. ✅ **Web app shows**: "⏳ Waiting for Device Confirmation"
2. ✅ **ESP32 LED turns ON**
3. ✅ **Serial monitor shows**: "💡 LED is ON - Press button to confirm"
4. **Press the button on ESP32**
5. ✅ **Serial monitor shows**: "✅ Button pressed - processing confirmation"
6. ✅ **Web app shows**: "Action 'Test Login Confirmation' confirmed successfully!"
7. ✅ **ESP32 LED turns OFF**

---

### Phase 6: Verification & Troubleshooting

#### 6.1 Verify Everything is Working
- ✅ Backend server running (check terminal)
- ✅ Frontend accessible at localhost:3001
- ✅ ESP32 connected to WiFi and MQTT
- ✅ Device registered in web app
- ✅ Button press triggers confirmation
- ✅ Real-time updates work

#### 6.2 Common Issues & Solutions

**🔧 ESP32 Issues:**

**Problem**: WiFi connection failed
```
❌ WiFi connection failed!
```
**Solution**: 
- Check WiFi name and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Try moving closer to router

**Problem**: MQTT connection failed
```
❌ MQTT connection failed, rc=-2
```
**Solution**:
- Check internet connection
- Verify firewall isn't blocking port 8883
- Try restarting ESP32

**🔧 Backend Issues:**

**Problem**: Port already in use
```
Error: listen EADDRINUSE: address already in use :::3000
```
**Solution**: 
```powershell
# Kill process using port 3000
netstat -ano | findstr :3000
taskkill /PID <PID_NUMBER> /F
```

**Problem**: Clerk authentication errors
```
ClerkError: Invalid publishable key
```
**Solution**:
- Verify API keys in `.env` file
- Ensure no extra spaces or quotes
- Check key starts with `pk_test_` or `pk_live_`

**🔧 Frontend Issues:**

**Problem**: Blank page or loading forever
**Solution**:
- Check browser console (F12)
- Verify Clerk publishable key in frontend `.env`
- Ensure backend is running

**🔧 Device Registration Issues:**

**Problem**: "Device already registered"
**Solution**:
- Device was registered before
- Go to "My Devices" tab to see existing devices
- Or restart ESP32 to generate new Device ID

**Problem**: "Invalid device secret format"  
**Solution**:
- Ensure secret is exactly 64 hex characters
- No spaces or line breaks
- Copy carefully from serial monitor

---

### Phase 7: Advanced Features & Customization

#### 7.1 Add More Devices
1. Flash firmware to another ESP32 (with different MAC)
2. Each device generates unique ID and secret
3. Register multiple devices in web app
4. Test confirmations on different devices

#### 7.2 Customize Actions
In the web app, try different action names:
- "Login to Banking"
- "Transfer $500"
- "Delete Account"
- "Admin Access"

#### 7.3 Hardware Improvements
- **Add buzzer**: Connect to GPIO pin, uncomment buzzer code
- **Add battery**: Use Li-Po + TP4056 charger for portability
- **3D print case**: Design enclosure for wearable form factor
- **Add more LEDs**: RGB LED for different status colors

#### 7.4 Security Enhancements
- **Hardware keys**: Use ESP32-H2/C61 for hardware-backed ECDSA
- **Certificate pinning**: Add proper TLS certificates
- **Rate limiting**: Prevent brute force attacks
- **Device attestation**: Verify device authenticity

---

### 🎯 Project Demonstration Tips

**For Class/Presentation:**
1. **Show the complete flow**: Web login → Request confirmation → Physical button → Success
2. **Explain security**: HMAC signatures, TLS transport, device secrets
3. **Demonstrate real-time updates**: WebSocket notifications
4. **Show hardware**: LED indication, button press, serial monitor
5. **Discuss scalability**: Multiple devices, production deployment

**Demo Script:**
1. "This is a secure 2FA wearable device"
2. "I log into the web app with Clerk authentication"
3. "When I request a sensitive action, it sends a challenge to my device"
4. "The device LED lights up, requiring physical confirmation"
5. "I press the button, device signs the challenge, server verifies"
6. "Real-time notification confirms the action is authenticated"

---

### 📚 Additional Resources

- **Clerk Documentation**: [https://clerk.com/docs](https://clerk.com/docs)
- **ESP32 Arduino Reference**: [https://docs.espressif.com/projects/arduino-esp32/](https://docs.espressif.com/projects/arduino-esp32/)
- **MQTT TLS Examples**: [https://docs.emqx.com/](https://docs.emqx.com/)
- **React Hooks Guide**: [https://reactjs.org/docs/hooks-intro.html](https://reactjs.org/docs/hooks-intro.html)

**🎓 This project demonstrates:**
- IoT device communication (MQTT over TLS)
- Modern web authentication (Clerk)
- Cryptographic signatures (HMAC-SHA256)
- Real-time updates (WebSockets)
- Full-stack development (React + Node.js)
- Hardware-software integration

**Perfect for coursework in:**
- IoT Security
- Embedded Systems
- Web Development
- Cryptography
- Human-Computer Interaction

---

**🔗 Need help?** Check the troubleshooting section above or review the serial monitor output for specific error messages.
