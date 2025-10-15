/*
 * ESP32 Hardware Self-Test
 * 
 * This program performs comprehensive hardware testing for:
 * 1. LED (GPIO2)
 * 2. BOOT button (GPIO0)
 * 3. WiFi (alternative approach)
 * 
 * No libraries required except built-in WiFi
 */

#include <WiFi.h>

// WiFi credentials - SET THESE CORRECTLY
const char* ssid = "madhur";
const char* password = "12345678";

// Hardware pins
const int LED_PIN = 2;
const int BUTTON_PIN = 0;

// Test states
enum TestState {
  TEST_LED,
  TEST_BUTTON,
  TEST_WIFI,
  TEST_COMPLETE
};

TestState currentTest = TEST_LED;
unsigned long testStartTime = 0;
bool testPassed = false;
bool buttonPressed = false;

void setup() {
  // Initialize serial with long timeout for slow serial monitors
  Serial.begin(115200);
  delay(3000);  // Wait 3 seconds for serial to connect
  
  Serial.println("\n\n");
  Serial.println("=====================================");
  Serial.println("   ESP32 HARDWARE SELF-TEST v1.0    ");
  Serial.println("=====================================");
  
  // Print device info
  Serial.print("ESP32 Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Set up pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Turn off LED initially
  digitalWrite(LED_PIN, LOW);
  
  // Start first test
  startLEDTest();
}

void loop() {
  // Run the current test
  switch (currentTest) {
    case TEST_LED:
      runLEDTest();
      break;
    case TEST_BUTTON:
      runButtonTest();
      break;
    case TEST_WIFI:
      runWiFiTest();
      break;
    case TEST_COMPLETE:
      runCompleteState();
      break;
  }
}

// ===== LED TEST FUNCTIONS =====

void startLEDTest() {
  Serial.println("\n----- LED TEST -----");
  Serial.println("Testing LED on GPIO2...");
  Serial.println("You should see the LED blinking for 5 seconds");
  
  testStartTime = millis();
  testPassed = false;
}

void runLEDTest() {
  // Blink LED for 5 seconds
  if (millis() - testStartTime < 5000) {
    // Blink pattern - 4 Hz (250ms period)
    if ((millis() % 250) < 125) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    // End of LED test
    digitalWrite(LED_PIN, LOW);
    
    // Print result prompt
    Serial.println("\nDid you see the LED blinking? (y/n)");
    Serial.println("Press the BOOT button to confirm YES");
    Serial.println("Wait 10 seconds for NO (timeout)");
    
    // Move to next stage - waiting for button press
    testStartTime = millis();
    currentTest = TEST_BUTTON;
  }
}

// ===== BUTTON TEST FUNCTIONS =====

void runButtonTest() {
  // Check for button press (with debouncing)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Button is pressed
      if (!buttonPressed) {
        buttonPressed = true;
        Serial.println("BOOT button pressed!");
        
        // Visual feedback - flash LED quickly
        for (int i = 0; i < 5; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(50);
          digitalWrite(LED_PIN, LOW);
          delay(50);
        }
        
        // LED test passed, button works!
        Serial.println("LED TEST PASSED ✅");
        Serial.println("BUTTON TEST PASSED ✅");
        
        // Move to WiFi test
        testStartTime = millis();
        testPassed = true;
        
        // Wait for button release
        while (digitalRead(BUTTON_PIN) == LOW) {
          delay(10);
        }
        Serial.println("Button released");
        
        // Start WiFi test
        currentTest = TEST_WIFI;
        startWiFiTest();
      }
    }
  } else {
    buttonPressed = false;
  }
  
  // Check for timeout after 10 seconds
  if (millis() - testStartTime > 10000 && !testPassed) {
    Serial.println("No button press detected - BUTTON TEST FAILED ❌");
    Serial.println("LED TEST FAILED ❌ (no confirmation)");
    
    // Move to WiFi test anyway
    currentTest = TEST_WIFI;
    startWiFiTest();
  }
}

// ===== WIFI TEST FUNCTIONS =====

void startWiFiTest() {
  Serial.println("\n----- WIFI TEST -----");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  // Disconnect any previous connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Set mode to station
  WiFi.mode(WIFI_STA);
  
  // Start connection
  WiFi.begin(ssid, password);
  
  testStartTime = millis();
  testPassed = false;
}

void runWiFiTest() {
  // Check WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    // Connected successfully
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    Serial.println("WIFI TEST PASSED ✅");
    testPassed = true;
    
    // Blink LED to indicate success
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    
    // Move to complete state
    currentTest = TEST_COMPLETE;
  }
  
  // Display progress dots
  static unsigned long lastDotTime = 0;
  if (millis() - lastDotTime > 500) {
    lastDotTime = millis();
    Serial.print(".");
  }
  
  // Check for timeout after 20 seconds
  if (millis() - testStartTime > 20000 && !testPassed) {
    Serial.println("\nWiFi connection failed after 20 seconds");
    Serial.print("WiFi status: ");
    printWiFiStatus(WiFi.status());
    Serial.println("WIFI TEST FAILED ❌");
    
    // Try alternative connection method
    Serial.println("\nTrying alternative WiFi connection method...");
    WiFi.disconnect(true, true);
    delay(1000);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    // Wait for another 20 seconds
    testStartTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - testStartTime < 20000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected with alternative method!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("WIFI TEST PASSED ✅");
      testPassed = true;
    } else {
      Serial.println("\nAll WiFi connection attempts failed");
      Serial.println("Possible issues:");
      Serial.println("1. Incorrect WiFi name or password");
      Serial.println("2. WiFi router is too far away");
      Serial.println("3. WiFi network is using incompatible security settings");
    }
    
    // Move to complete state regardless of outcome
    currentTest = TEST_COMPLETE;
  }
}

// ===== COMPLETION FUNCTIONS =====

void runCompleteState() {
  static bool resultsPrinted = false;
  
  if (!resultsPrinted) {
    resultsPrinted = true;
    
    Serial.println("\n----- TEST SUMMARY -----");
    Serial.println("Hardware test completed!");
    
    // Final LED pattern based on test results
    if (testPassed) {
      // Success pattern - triple blink every 2 seconds
      while (true) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(100);
          digitalWrite(LED_PIN, LOW);
          delay(100);
        }
        delay(2000);
      }
    } else {
      // Error pattern - long blink every 2 seconds
      while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(1000);
        digitalWrite(LED_PIN, LOW);
        delay(1000);
      }
    }
  }
}

// ===== UTILITY FUNCTIONS =====

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