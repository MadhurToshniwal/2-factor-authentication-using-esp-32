/*
 * ESP32 Device Info Display
 * Shows Device ID and generates a sample secret for registration
 */

#include <WiFi.h>
#include <Preferences.h>
#include "esp_random.h"

String deviceId;
uint8_t secretKey[32];
Preferences preferences;

// Convert binary data to hex string
String hexEncode(const uint8_t *buffer, size_t length) {
  String result;
  result.reserve(length * 2);
  const char hexChars[] = "0123456789abcdef";
  
  for (size_t i = 0; i < length; i++) {
    result += hexChars[(buffer[i] >> 4) & 0x0F];
    result += hexChars[buffer[i] & 0x0F];
  }
  
  return result;
}

// Generate secure random bytes
void generateSecureRandom(uint8_t* buffer, size_t length) {
  for (size_t i = 0; i < length; i++) {
    uint32_t randomValue = esp_random();
    buffer[i] = (uint8_t)(randomValue & 0xFF);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n" + String("=").substring(0, 50));
  Serial.println("    ESP32 DEVICE REGISTRATION INFO");
  Serial.println(String("=").substring(0, 50));
  
  // Get device ID from MAC address
  deviceId = WiFi.macAddress();
  Serial.println("📱 Device ID: " + deviceId);
  
  // Generate or load secret key
  preferences.begin("2fa-device", false);
  
  if (preferences.getBytesLength("secret") == 32) {
    preferences.getBytes("secret", secretKey, 32);
    Serial.println("📋 Using existing secret key");
  } else {
    generateSecureRandom(secretKey, 32);
    preferences.putBytes("secret", secretKey, 32);
    Serial.println("🔐 Generated new secret key");
  }
  
  preferences.end();
  
  String secretHex = hexEncode(secretKey, 32);
  
  Serial.println("\n" + String("=").substring(0, 50));
  Serial.println("  📋 COPY THIS INFO TO WEB APP:");
  Serial.println(String("=").substring(0, 50));
  Serial.println("Device ID: " + deviceId);
  Serial.println("Secret Key: " + secretHex);
  Serial.println("Device Name: ESP32 2FA Band");
  Serial.println(String("=").substring(0, 50));
  Serial.println("✅ Ready for registration!");
  Serial.println("🌐 Go to: http://localhost:3001");
  Serial.println("📱 Navigate to 'Devices' page");
  Serial.println("➕ Click 'Add Device'");
  Serial.println("📝 Paste the info above");
  Serial.println(String("=").substring(0, 50));
}

void loop() {
  // Blink LED to show device is ready
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  delay(1000);
  
  // Print reminder every 10 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 10000) {
    Serial.println("💡 Device ready for registration - Device ID: " + deviceId);
    lastPrint = millis();
  }
}