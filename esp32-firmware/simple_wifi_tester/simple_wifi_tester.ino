/*
 * ESP32 Simple WiFi Connection Tester
 * 
 * This program systematically tests WiFi connectivity with your network
 * using simpler methods that should avoid compilation errors.
 */

#include <WiFi.h>

// ===== WIFI CONFIGURATION =====
const char* primary_ssid = "madhur";
const char* primary_password = "12345678";

// LED pin for status indication
const int LED_PIN = 2;

// Test status
int testNumber = 1;
unsigned long testStartTime = 0;
bool connectionSuccess = false;

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(2000);  // Wait for serial connection
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  
  // Print welcome message
  Serial.println("\n\n========================================");
  Serial.println("   ESP32 Simple WiFi Connection Tester   ");
  Serial.println("========================================");
  
  // Print device info
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Flash LED to indicate start
  flashLED(3, 200);
  
  // Prepare for first test
  Serial.println("\nStarting WiFi tests. We'll try several methods:");
  Serial.println("1. Standard connection");
  Serial.println("2. Connection with 1-second delay before connect");
  Serial.println("3. Connection with WiFi.disconnect before connect");
  Serial.println("4. Connection with auto-reconnect enabled");
  Serial.println("5. Connection with channel 1 forced");
  
  // Start first test
  startNextTest();
}

void loop() {
  // If we're testing a connection method
  if (!connectionSuccess) {
    // Check connection progress
    checkConnectionProgress();
  } else {
    // If connected, just maintain the connection
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi connection lost!");
      connectionSuccess = false;
      testNumber++;
      startNextTest();
    }
    
    // Blink LED occasionally to show we're still connected
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 3000) {
      flashLED(1, 100);
      lastBlink = millis();
    }
  }
}

void startNextTest() {
  // Disconnect any previous connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Reset WiFi
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  // Print test info
  Serial.println("\n----------------------------------------");
  Serial.print("TEST #");
  Serial.print(testNumber);
  Serial.print(": ");
  
  // Configure the current test
  switch (testNumber) {
    case 1:
      Serial.println("Standard connection method");
      WiFi.begin(primary_ssid, primary_password);
      break;
      
    case 2:
      Serial.println("Connection with delay");
      delay(1000);  // Add an extra delay
      WiFi.begin(primary_ssid, primary_password);
      break;
      
    case 3:
      Serial.println("Connection with explicit disconnect");
      WiFi.disconnect(true);
      delay(1000);
      WiFi.begin(primary_ssid, primary_password);
      break;
      
    case 4:
      Serial.println("Connection with auto-reconnect");
      WiFi.setAutoReconnect(true);
      WiFi.begin(primary_ssid, primary_password);
      break;
      
    case 5:
      Serial.println("Connection with channel 1");
      WiFi.begin(primary_ssid, primary_password, 1);  // Force channel 1
      break;
      
    default:
      // We've tried all tests, start over from test 1
      testNumber = 1;
      Serial.println("\n======= RESTARTING TESTS =======");
      Serial.println("All tests completed without success.");
      Serial.println("Starting over with test #1");
      WiFi.begin(primary_ssid, primary_password);
      break;
  }
  
  Serial.print("Attempting to connect to: ");
  Serial.println(primary_ssid);
  
  // Record test start time
  testStartTime = millis();
}

void checkConnectionProgress() {
  // Display progress dots
  static unsigned long lastDot = 0;
  if (millis() - lastDot > 500) {
    Serial.print(".");
    lastDot = millis();
    
    // Toggle LED to show activity
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  
  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    // Success!
    Serial.println();
    Serial.println("✓ WiFi CONNECTED SUCCESSFULLY!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Print success message with test number
    Serial.print("\n*** SUCCESS WITH TEST #");
    Serial.print(testNumber);
    Serial.println(" ***");
    
    // Remember we're connected
    connectionSuccess = true;
    
    // Blink LED rapidly to indicate success
    digitalWrite(LED_PIN, LOW);
    flashLED(5, 100);
    
    return;
  }
  
  // Check for timeout (15 seconds per test)
  if (millis() - testStartTime > 15000) {
    Serial.println();
    Serial.print("✗ Test #");
    Serial.print(testNumber);
    Serial.print(" failed. WiFi status: ");
    printWiFiStatus(WiFi.status());
    
    // Move to next test
    testNumber++;
    startNextTest();
  }
}

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

void flashLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}