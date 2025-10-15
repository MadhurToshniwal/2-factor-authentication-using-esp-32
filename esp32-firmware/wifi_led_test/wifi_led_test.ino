/*
 * ESP32 WiFi + LED Test
 * 
 * This program tests basic connectivity and LED functionality.
 * It will:
 * 1. Flash the LED to confirm hardware works
 * 2. Try to connect to WiFi
 * 3. Indicate connection status with the LED
 */

#include <WiFi.h>
#include "esp_wifi.h"

// WiFi credentials - CHANGE THESE!
const char* ssid = "madhur";     // Replace with your actual WiFi name
const char* password = "12345678";  // Replace with your actual WiFi password

// LED pin on ESP32 ThingzMini
const int LED_PIN = 2;

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 ThingzMini WiFi + LED Test ===");
  
  // Set up LED
  pinMode(LED_PIN, OUTPUT);
  
  // Flash LED 3 times to confirm hardware
  Serial.println("Testing LED...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  Serial.println("LED test complete");
  
  // Connect to WiFi
  connectToWiFi();
}

void loop() {
  // If connected, flash LED once every 3 seconds
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(2900);
  } 
  // If not connected, flash LED rapidly to indicate error
  else {
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    delay(1000);
    
    // Try to reconnect
    connectToWiFi();
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  
  // Disconnect from any previous connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  
  // Disable power saving for better stability
  esp_wifi_set_ps(WIFI_PS_NONE);
  
  // Begin connection
  WiFi.begin(ssid, password);
  
  // Flash LED slowly while connecting
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  
  // Check connection result
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Flash LED rapidly to indicate success
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  } else {
    Serial.println("Failed to connect to WiFi!");
  }
}
