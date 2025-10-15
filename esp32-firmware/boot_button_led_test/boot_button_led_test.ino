/*
 * ESP32 ThingzMini BOOT Button + LED Test
 * 
 * This program tests the BOOT button (GPIO0) and LED (GPIO2) functionality.
 * It also includes alternative WiFi connection methods.
 * 
 * Features:
 * 1. LED will turn on for 3 seconds at startup
 * 2. Pressing the BOOT button will toggle the LED state
 * 3. Multiple WiFi connection approaches are tried
 * 4. Serial monitor shows detailed diagnostics
 */

#include <WiFi.h>
#include "esp_wifi.h"

// WiFi credentials - CHANGE THESE!
const char* ssid = "madhur";           // Your WiFi name
const char* password = "12345678";     // Your WiFi password

// Hardware pins
const int BOOT_BUTTON_PIN = 0;  // BOOT button is on GPIO0
const int LED_PIN = 2;          // LED is on GPIO2

// Global variables
bool ledState = false;
unsigned long lastButtonCheck = 0;
unsigned long lastWifiAttempt = 0;
int wifiAttemptCount = 0;
int connectionMethod = 0;  // We'll try different connection methods

void setup() {
  // Initialize serial with higher timeout
  Serial.begin(115200);
  delay(2000);  // Allow time for serial to initialize
  
  Serial.println("\n\n=== ESP32 ThingzMini BOOT Button + LED Test ===");
  Serial.println("Press the BOOT button to toggle the LED");
  
  // Set up pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);  // BOOT button is active LOW
  
  // Turn LED on for 3 seconds to verify it works
  Serial.println("Turning LED ON for 3 seconds...");
  digitalWrite(LED_PIN, HIGH);
  delay(3000);
  digitalWrite(LED_PIN, LOW);
  
  // Print some debug info
  Serial.println("\nHardware Information:");
  Serial.println("---------------------");
  Serial.print("ESP32 Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("ESP32 SDK Version: ");
  Serial.println(ESP.getSdkVersion());
  Serial.print("ESP32 Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Try first WiFi connection method
  connectToWiFi();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check BOOT button with debouncing (every 50ms)
  if (currentMillis - lastButtonCheck >= 50) {
    lastButtonCheck = currentMillis;
    
    // Check if button is pressed (active LOW)
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
      // Wait for button release with debounce
      delay(50);
      
      // If still pressed after debounce
      if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        // Toggle LED state
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        
        Serial.print("BOOT button pressed! LED turned ");
        Serial.println(ledState ? "ON" : "OFF");
        
        // Wait for button release to prevent multiple toggles
        while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
          delay(10);
        }
      }
    }
  }
  
  // Check WiFi status and try reconnecting if needed
  if (WiFi.status() != WL_CONNECTED) {
    // If not connected and 15 seconds have passed since last attempt
    if (currentMillis - lastWifiAttempt >= 15000) {
      lastWifiAttempt = currentMillis;
      wifiAttemptCount++;
      
      // After 3 failed attempts with one method, try another method
      if (wifiAttemptCount >= 3) {
        wifiAttemptCount = 0;
        connectionMethod = (connectionMethod + 1) % 3;
        Serial.print("\nTrying WiFi connection method #");
        Serial.println(connectionMethod + 1);
      }
      
      connectToWiFi();
    }
    
    // When not connected, blink LED rapidly to indicate no connection
    // (unless user has manually turned it on with button)
    if (!ledState) {
      if ((currentMillis % 1000) < 200) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
    }
  } else {
    // If WiFi is connected and LED wasn't manually set by user
    if (!ledState) {
      // Slow pulse when connected
      if ((currentMillis % 3000) < 200) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
}

void connectToWiFi() {
  Serial.print("\nAttempting to connect to WiFi: ");
  Serial.println(ssid);
  
  // Disconnect from any previous WiFi
  WiFi.disconnect(true);
  delay(1000);
  
  // Always set mode to station
  WiFi.mode(WIFI_STA);
  
  // Try different connection methods
  switch (connectionMethod) {
    case 0:
      // Method 1: Standard approach with power saving disabled
      Serial.println("Method 1: Standard connection with power saving disabled");
      esp_wifi_set_ps(WIFI_PS_NONE);
      WiFi.begin(ssid, password);
      break;
      
    case 1:
      // Method 2: With specific WiFi config
      Serial.println("Method 2: Connection with specific WiFi config");
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      WiFi.begin(ssid, password);
      break;
      
    case 2:
      // Method 3: With different channel settings
      Serial.println("Method 3: Connection with low-level configuration");
      WiFi.disconnect(true, true);  // Disconnect and clear saved settings
      delay(1000);
      WiFi.persistent(false);
      // Try to connect with specific parameters
      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      esp_wifi_init(&cfg);
      esp_wifi_set_mode(WIFI_MODE_STA);
      esp_wifi_start();
      WiFi.begin(ssid, password);
      break;
  }
  
  // Print MAC address, which can be useful for debugging
  Serial.print("Device MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Wait for connection with timeout
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < 10000) {  // 10 second timeout
    
    Serial.print(".");
    delay(500);
    
    // Flash LED faster during connection attempt
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
  Serial.println();
  
  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Blink LED rapidly to indicate successful connection
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  } else {
    Serial.println("Failed to connect to WiFi!");
    Serial.print("WiFi status: ");
    printWiFiStatus(WiFi.status());
    
    // Blink LED three times slowly to indicate failure
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  }
}

// Helper function to print WiFi status in human-readable form
void printWiFiStatus(int status) {
  switch (status) {
    case WL_IDLE_STATUS:
      Serial.println("IDLE");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("NO SSID AVAILABLE");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("SCAN COMPLETED");
      break;
    case WL_CONNECTED:
      Serial.println("CONNECTED");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("CONNECTION FAILED");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("CONNECTION LOST");
      break;
    case WL_DISCONNECTED:
      Serial.println("DISCONNECTED");
      break;
    default:
      Serial.print("UNKNOWN (");
      Serial.print(status);
      Serial.println(")");
      break;
  }
}