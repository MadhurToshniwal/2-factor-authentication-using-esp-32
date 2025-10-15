/*
 * Secure 2FA Confirmation Band - ESP32 Firmware
 * 
 * This firmware implements a secure wearable 2FA device that:
 * 1. Generates a secure random key on first boot
 * 2. Connects to WiFi and MQTT broker over TLS
 * 3. Listens for confirmation challenges
 * 4. Waits for physical button press to confirm
 * 5. Signs challenges with HMAC-SHA256 and sends back
 * 
 * Hardware:
 * - ESP32 development board
 * - Push button connected to GPIO 0 (with internal pullup)
 * - LED connected to GPIO 2
 * - Optional: Buzzer/vibration motor for feedback
 * 
 * Author: Generated for Secure 2FA Band Project
 * License: MIT
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "mbedtls/md.h"
#include "esp_random.h"

// ==================== CONFIGURATION ====================
// WiFi Configuration
const char* ssid = "madhur";     // Your WiFi network name
const char* password = "12345678";  // Your WiFi password

// MQTT Configuration
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 8883;

// For EMQX public broker, we'll skip certificate validation for simplicity
// In production, include the proper CA certificate
const char* ca_cert = nullptr; // Using nullptr to skip cert validation

// Hardware Configuration
const int buttonPin = 0;    // GPIO0 - usually has built-in pullup
const int ledPin = 2;       // GPIO2 - built-in LED on most ESP32 boards
const int buzzerPin = -1;   // Set to valid GPIO pin if you have a buzzer

// Device Configuration
String deviceId;
uint8_t secretKey[32];
const int secretLen = 32;
String deviceName = "ESP32 2FA Band";

// ==================== GLOBAL VARIABLES ====================
WiFiClientSecure espClient;
PubSubClient client(espClient);
Preferences preferences;

unsigned long lastWiFiCheck = 0;
unsigned long lastMQTTCheck = 0;
const unsigned long wifiCheckInterval = 30000;  // 30 seconds
const unsigned long mqttCheckInterval = 10000;  // 10 seconds

bool deviceRegistered = false;
bool waitingForConfirmation = false;
String currentChallenge = "";
String currentConfirmationId = "";

// ==================== UTILITY FUNCTIONS ====================

// Generate secure random bytes using ESP32's hardware RNG
void generateSecureRandom(uint8_t* buffer, size_t length) {
  for (size_t i = 0; i < length; i++) {
    uint32_t randomValue = esp_random();
    buffer[i] = (uint8_t)(randomValue & 0xFF);
  }
}

// Convert binary data to hex string
String hexEncode(const uint8_t *buffer, size_t length) {
  String result;
  result.reserve(length * 2);
  const char hexChars[] = "0123456789abcdef";
  
  for (size_t i = 0; i < length; i++) {
    // Always output exactly 2 hex characters per byte
    result += hexChars[(buffer[i] >> 4) & 0x0F];
    result += hexChars[buffer[i] & 0x0F];
  }
  
  // For 32-byte secret, this will always produce exactly 64 characters
  // No padding needed - each byte becomes exactly 2 hex chars
  return result;
}

// Initialize or load device secret from NVS
void initializeDeviceSecret() {
  preferences.begin("2fa-device", false);
  
  // Force regenerate secret for proper format (remove this check after first successful registration)
  // Check if secret already exists
  if (false && preferences.getBytesLength("secret") == secretLen) {
    // Load existing secret
    preferences.getBytes("secret", secretKey, secretLen);
    Serial.println("✅ Loaded existing device secret from NVS");
  } else {
    // Generate new secret
    generateSecureRandom(secretKey, secretLen);
    
    // Save to NVS
    preferences.putBytes("secret", secretKey, secretLen);
    Serial.println("🔐 Generated and saved new device secret");
  }
  
  preferences.end();
}

// HMAC-SHA256 computation
bool computeHMAC(const String& message, const uint8_t* key, size_t keyLen, uint8_t* output) {
  const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (md_info == nullptr) {
    Serial.println("❌ Failed to get SHA256 MD info");
    return false;
  }
  
  int result = mbedtls_md_hmac(
    md_info,
    key, keyLen,
    (const unsigned char*)message.c_str(), message.length(),
    output
  );
  
  if (result != 0) {
    Serial.printf("❌ HMAC computation failed: %d\n", result);
    return false;
  }
  
  return true;
}

// ==================== HARDWARE FUNCTIONS ====================

void initializeHardware() {
  // Initialize button with internal pullup
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Initialize LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Initialize buzzer if connected
  if (buzzerPin >= 0) {
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
  }
  
  Serial.println("🔧 Hardware initialized");
}

void indicateStatus(bool waiting) {
  if (waiting) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void playFeedback(bool success) {
  if (buzzerPin >= 0) {
    if (success) {
      // Success tone
      digitalWrite(buzzerPin, HIGH);
      delay(100);
      digitalWrite(buzzerPin, LOW);
      delay(50);
      digitalWrite(buzzerPin, HIGH);
      delay(100);
      digitalWrite(buzzerPin, LOW);
    } else {
      // Error tone
      digitalWrite(buzzerPin, HIGH);
      delay(300);
      digitalWrite(buzzerPin, LOW);
    }
  }
}

bool waitForButtonPress(unsigned long timeout = 300000) { // 5 minute timeout
  unsigned long startTime = millis();
  
  while (millis() - startTime < timeout) {
    if (digitalRead(buttonPin) == LOW) { // Button pressed (active low)
      delay(50); // Debounce
      if (digitalRead(buttonPin) == LOW) {
        // Wait for release
        while (digitalRead(buttonPin) == LOW) {
          delay(10);
        }
        return true;
      }
    }
    
    // Keep MQTT connection alive while waiting
    if (!client.connected()) {
      connectMQTT();
    }
    client.loop();
    
    delay(10);
  }
  
  return false; // Timeout
}

// ==================== WIFI FUNCTIONS ====================

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  
  Serial.print("🔄 Connecting to WiFi: ");
  Serial.println(ssid);
  
  // Disconnect first to clear any previous state
  WiFi.disconnect(true);
  delay(1000);
  
  // Reset WiFi mode
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  // Disable sleep mode for better connection stability
  WiFi.setSleep(false);
  
  // Begin connection
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  int lastStatus = WL_IDLE_STATUS;
  
  Serial.print("Waiting for connection");
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 60000) {  // Increased timeout to 60 seconds
    delay(500);
    Serial.print(".");
    
    // Check if status changed
    int currentStatus = WiFi.status();
    if (currentStatus != lastStatus) {
      lastStatus = currentStatus;
      Serial.println();
      Serial.print("Status changed to: ");
      
      switch (currentStatus) {
        case WL_IDLE_STATUS:
          Serial.println("IDLE");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.println("NO SSID AVAILABLE - Network not found");
          // Try different connection method on specific failures
          WiFi.disconnect(true);
          delay(1000);
          WiFi.mode(WIFI_STA);
          WiFi.begin(ssid, password);
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("SCAN COMPLETED");
          break;
        case WL_CONNECTED:
          Serial.println("CONNECTED");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("CONNECTION FAILED - Password may be incorrect");
          // Try different connection method on specific failures
          WiFi.disconnect(true);
          delay(1000);
          WiFi.mode(WIFI_STA);
          WiFi.begin(ssid, password);
          break;
        case WL_CONNECTION_LOST:
          Serial.println("CONNECTION LOST");
          break;
        case WL_DISCONNECTED:
          Serial.println("DISCONNECTED");
          break;
        default:
          Serial.print("UNKNOWN (");
          Serial.print(currentStatus);
          Serial.println(")");
          break;
      }
      lastStatus = WiFi.status();
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi connected!");
    Serial.print("📍 IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
    Serial.print("Last status: ");
    
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("NO SSID AVAILABLE - Network not found");
        Serial.println("Check if your router is powered on and broadcasting.");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("CONNECTION FAILED - Password may be incorrect");
        Serial.println("Verify your WiFi password is correct.");
        break;
      case WL_DISCONNECTED:
        Serial.println("DISCONNECTED");
        Serial.println("Try moving closer to your router.");
        break;
      default:
        Serial.print("UNKNOWN (");
        Serial.print(WiFi.status());
        Serial.println(")");
        break;
    }
  }
}

// ==================== MQTT FUNCTIONS ====================

void connectMQTT() {
  if (client.connected()) {
    return;
  }
  
  // Configure TLS (skip certificate validation for demo)
  espClient.setInsecure(); // For production, use proper certificates
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(onMqttMessage);
  client.setKeepAlive(60);
  
  Serial.print("🔄 Connecting to MQTT broker: ");
  Serial.println(mqtt_server);
  
  String clientId = "esp32-2fa-" + deviceId;
  
  unsigned long startTime = millis();
  while (!client.connected() && millis() - startTime < 15000) {
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ MQTT connected!");
      
      // Subscribe to challenge topic
      String challengeTopic = "devices/" + deviceId + "/challenge";
      if (client.subscribe(challengeTopic.c_str())) {
        Serial.println("📡 Subscribed to challenge topic: " + challengeTopic);
      } else {
        Serial.println("❌ Failed to subscribe to challenge topic");
      }
      
      // Publish online status
      String statusTopic = "devices/" + deviceId + "/status";
      client.publish(statusTopic.c_str(), "online", true);
      
    } else {
      Serial.print("❌ MQTT connection failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
  
  if (!client.connected()) {
    Serial.println("❌ MQTT connection timeout!");
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.printf("📥 MQTT message received on topic: %s\n", topic);
  Serial.println("📄 Payload: " + message);
  
  // Parse topic
  String topicStr = String(topic);
  if (topicStr.endsWith("/challenge")) {
    handleChallengeMessage(message);
  }
}

void handleChallengeMessage(const String& message) {
  // Parse JSON message
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("❌ Failed to parse challenge JSON");
    return;
  }
  
  if (!doc.containsKey("challenge") || !doc.containsKey("confirmationId")) {
    Serial.println("❌ Invalid challenge format");
    return;
  }
  
  currentChallenge = doc["challenge"].as<String>();
  currentConfirmationId = doc["confirmationId"].as<String>();
  String action = doc["action"].as<String>();
  
  Serial.println("🔔 Challenge received!");
  Serial.println("Action: " + action);
  Serial.println("Challenge ID: " + currentConfirmationId);
  
  // Indicate waiting for user confirmation
  waitingForConfirmation = true;
  indicateStatus(true);
  
  Serial.println("💡 LED is ON - Press button to confirm action: " + action);
  
  // Wait for button press in background
  bool confirmed = waitForButtonPress();
  
  if (confirmed) {
    Serial.println("✅ Button pressed - processing confirmation");
    processConfirmation();
  } else {
    Serial.println("⏰ Confirmation timeout");
    waitingForConfirmation = false;
    indicateStatus(false);
  }
}

void processConfirmation() {
  if (currentChallenge.isEmpty() || currentConfirmationId.isEmpty()) {
    Serial.println("❌ No valid challenge to process");
    return;
  }
  
  // Compute HMAC signature
  uint8_t signature[32];
  if (!computeHMAC(currentChallenge, secretKey, secretLen, signature)) {
    Serial.println("❌ Failed to compute HMAC signature");
    playFeedback(false);
    waitingForConfirmation = false;
    indicateStatus(false);
    return;
  }
  
  String signatureHex = hexEncode(signature, 32);
  
  // Create response JSON
  DynamicJsonDocument responseDoc(512);
  responseDoc["signatureHex"] = signatureHex;
  responseDoc["confirmationId"] = currentConfirmationId;
  responseDoc["timestamp"] = millis();
  
  String responseMessage;
  serializeJson(responseDoc, responseMessage);
  
  // Publish response
  String responseTopic = "devices/" + deviceId + "/response";
  if (client.publish(responseTopic.c_str(), responseMessage.c_str())) {
    Serial.println("✅ Confirmation sent successfully!");
    Serial.println("Response: " + responseMessage);
    playFeedback(true);
  } else {
    Serial.println("❌ Failed to send confirmation");
    playFeedback(false);
  }
  
  // Reset state
  waitingForConfirmation = false;
  indicateStatus(false);
  currentChallenge = "";
  currentConfirmationId = "";
}

// ==================== SETUP & LOOP ====================

void setup() {
  Serial.begin(115200);
  delay(3000);  // Increased delay for serial initialization
  
  Serial.println("\n=== ESP32 STARTING ===");
  Serial.println("🔐 Secure 2FA Confirmation Band");
  Serial.println("==================================");
  
  // Initialize hardware
  initializeHardware();
  
  // Generate device ID from MAC address
  WiFi.mode(WIFI_STA);  // Initialize WiFi mode first to get MAC
  deviceId = WiFi.macAddress();
  Serial.println("🆔 Device ID: " + deviceId);
  
  // Initialize or load device secret
  initializeDeviceSecret();
  
  // Print pairing information for initial setup
  String deviceSecretHex = hexEncode(secretKey, secretLen);
  Serial.println("\n📋 DEVICE PAIRING INFORMATION");
  Serial.println("=====================================");
  Serial.println("Device ID: " + deviceId);
  Serial.println("Device Secret (hex): " + deviceSecretHex);
  Serial.println("Secret Length: " + String(deviceSecretHex.length()) + " characters");
  Serial.println("Device Name: " + deviceName);
  Serial.println("=====================================");
  Serial.println("📱 Copy the above information to register this device in the web app");
  Serial.println("🔗 Make sure you're logged into the web app first!");
  Serial.println("✅ Secret format is " + String(deviceSecretHex.length() == 64 ? "VALID" : "INVALID"));
  Serial.println("");
  
  // Connect to WiFi
  connectWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    // Connect to MQTT
    connectMQTT();
    
    if (client.connected()) {
      Serial.println("🚀 Device ready for 2FA confirmations!");
      Serial.println("💡 LED will light up when confirmation is needed");
      Serial.println("🔘 Press button to confirm actions");
      
      // Brief LED flash to indicate ready
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(200);
        digitalWrite(ledPin, LOW);
        delay(200);
      }
    }
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check WiFi connection periodically
  if (currentTime - lastWiFiCheck > wifiCheckInterval) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("📡 WiFi disconnected, reconnecting...");
      connectWiFi();
    }
    lastWiFiCheck = currentTime;
  }
  
  // Check MQTT connection periodically
  if (currentTime - lastMQTTCheck > mqttCheckInterval) {
    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
      Serial.println("📡 MQTT disconnected, reconnecting...");
      connectMQTT();
    }
    lastMQTTCheck = currentTime;
  }
  
  // Process MQTT messages
  if (client.connected()) {
    client.loop();
  }
  
  // Handle button press for status check (when not waiting for confirmation)
  if (!waitingForConfirmation && digitalRead(buttonPin) == LOW) {
    delay(50); // Debounce
    if (digitalRead(buttonPin) == LOW) {
      // Button held - show status
      Serial.println("\n📊 DEVICE STATUS");
      Serial.println("================");
      Serial.println("Device ID: " + deviceId);
      Serial.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
      Serial.println("MQTT: " + String(client.connected() ? "Connected" : "Disconnected"));
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("Signal: " + String(WiFi.RSSI()) + " dBm");
      Serial.println("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
      Serial.println("================\n");
      
      // Flash LED to acknowledge button press
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      
      // Wait for button release
      while (digitalRead(buttonPin) == LOW) {
        delay(10);
      }
    }
  }
  
  delay(10); // Small delay to prevent watchdog issues
}
