/*
 * ESP32 WiFi Connection Solver
 * 
 * This program tries multiple methods to connect to WiFi networks
 * that might be challenging for ESP32 devices to connect to.
 * 
 * It includes:
 * - Multiple connection methods
 * - Detailed diagnostics
 * - Mobile hotspot alternative
 * - Static IP configuration option
 */

#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

// ===== WIFI CONFIGURATION =====

// Primary WiFi - Your home network
const char* primary_ssid = "madhur";
const char* primary_password = "12345678";

// Alternative WiFi - Try creating a mobile hotspot with these settings
const char* alt_ssid = "ESP32_Hotspot";
const char* alt_password = "12345678";

// Static IP Configuration (optional)
// If your router has issues with DHCP, set useStaticIP to true and configure below
const bool useStaticIP = false;
IPAddress staticIP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

// LED pin for status indication
const int LED_PIN = 2;

// ===== VARIABLES =====
int connectionMethod = 0;
int networkSelection = 0;
unsigned long attemptStartTime = 0;
bool connectionSuccess = false;

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(3000);  // Wait for serial connection
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  
  // Print welcome message
  Serial.println("\n\n=================================================");
  Serial.println("      ESP32 WiFi Connection Solver v1.0");
  Serial.println("=================================================");
  
  // Print device info
  Serial.print("ESP32 Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Start with primary network
  Serial.println("\nStarting WiFi tests with primary network");
  networkSelection = 0;
  
  // Flash LED to indicate start
  flashLED(3, 200);
  
  // Start first connection attempt
  startNextConnectionAttempt();
}

void loop() {
  // Check connection progress
  if (!connectionSuccess) {
    checkConnectionProgress();
  } else {
    // If connected, just keep the connection alive and blink the LED
    static unsigned long lastBlinkTime = 0;
    if (millis() - lastBlinkTime > 3000) {
      lastBlinkTime = millis();
      flashLED(1, 100);
    }
    
    // Check if connection is still active
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n⚠️ WiFi connection lost! Reconnecting...");
      connectionSuccess = false;
      startNextConnectionAttempt();
    }
  }
}

void startNextConnectionAttempt() {
  // Disconnect any previous connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Select network
  const char* current_ssid = (networkSelection == 0) ? primary_ssid : alt_ssid;
  const char* current_password = (networkSelection == 0) ? primary_password : alt_password;
  
  // Print connection attempt info
  Serial.println("\n-------------------------------------------------");
  Serial.print("📶 Attempting to connect to: ");
  Serial.println(current_ssid);
  Serial.print("🔄 Using connection method #");
  Serial.println(connectionMethod + 1);
  
  // Set WiFi mode to station
  WiFi.mode(WIFI_STA);
  
  // Configure connection based on method
  switch (connectionMethod) {
    case 0:
      // Method 1: Standard approach with power saving disabled
      Serial.println("Method 1: Standard connection with power saving disabled");
      esp_wifi_set_ps(WIFI_PS_NONE);
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password);
      break;
      
    case 1:
      // Method 2: With specific WiFi config
      Serial.println("Method 2: Connection with persistent settings disabled");
      WiFi.persistent(false);
      WiFi.setAutoReconnect(true);
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password);
      break;
      
    case 2: {
      // Method 3: With low-level initialization
      Serial.println("Method 3: Connection with full reset and low-level initialization");
      WiFi.disconnect(true, true);  // Disconnect and clear saved settings
      delay(1000);
      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      esp_wifi_init(&cfg);
      esp_wifi_set_mode(WIFI_MODE_STA);
      esp_wifi_start();
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password);
      break;
    }
      
    case 3:
      // Method 4: With channel specification (try channel 1)
      Serial.println("Method 4: Connection with channel 1 specification");
      WiFi.disconnect(true, true);
      delay(1000);
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password, 1); // Force channel 1
      break;
      
    case 4:
      // Method 5: With channel specification (try channel 6)
      Serial.println("Method 5: Connection with channel 6 specification");
      WiFi.disconnect(true, true);
      delay(1000);
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password, 6); // Force channel 6
      break;
      
    case 5:
      // Method 6: With channel specification (try channel 11)
      Serial.println("Method 6: Connection with channel 11 specification");
      WiFi.disconnect(true, true);
      delay(1000);
      if (useStaticIP) {
        WiFi.config(staticIP, gateway, subnet, dns);
      }
      WiFi.begin(current_ssid, current_password, 11); // Force channel 11
      break;
  }
  
  // Record start time
  attemptStartTime = millis();
  
  // Print MAC address, which can be useful for debugging
  Serial.print("Device MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void checkConnectionProgress() {
  // Display progress dots
  static unsigned long lastProgressTime = 0;
  if (millis() - lastProgressTime > 500) {
    lastProgressTime = millis();
    Serial.print(".");
    
    // Blink LED to show we're trying
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  
  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    // Success!
    Serial.println();
    Serial.println("✅ WiFi CONNECTED SUCCESSFULLY!");
    Serial.print("📱 Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("📶 Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("🔒 Channel: ");
    Serial.println(WiFi.channel());
    Serial.print("🌐 IP address: ");
    Serial.println(WiFi.localIP());
    
    // Check if mDNS works
    if (MDNS.begin("esp32")) {
      Serial.println("✅ mDNS responder started");
      Serial.println("   Device can be reached at: esp32.local");
    }
    
    // Connection details for troubleshooting
    Serial.println("\n----- Connection Method That Worked -----");
    Serial.print("Network: ");
    Serial.println(networkSelection == 0 ? "Primary" : "Alternative");
    Serial.print("Method: #");
    Serial.println(connectionMethod + 1);
    
    // Recommendations
    Serial.println("\nRECOMMENDATION: Use this connection method in your main program!");
    Serial.println("Update your main code with:");
    Serial.println("1. Use the same WiFi credentials");
    Serial.println("2. Copy the connection method that worked");
    
    // Remember that we're connected
    connectionSuccess = true;
    
    // Show success with LED
    digitalWrite(LED_PIN, LOW);
    flashLED(5, 100);
    
    return;
  }
  
  // Check if attempt timed out (20 seconds per attempt)
  if (millis() - attemptStartTime > 20000) {
    Serial.println();
    Serial.print("❌ Connection attempt failed: ");
    printWiFiStatus(WiFi.status());
    
    // Try next method
    connectionMethod = (connectionMethod + 1) % 6;
    
    // If we've tried all methods with current network, switch to alternative
    if (connectionMethod == 0) {
      networkSelection = (networkSelection + 1) % 2;
      
      if (networkSelection == 1) {
        Serial.println("\n⚠️ All methods failed with primary network. Switching to alternative network.");
        Serial.println("📱 IMPORTANT: Please create a mobile hotspot with the following credentials:");
        Serial.println("   SSID: " + String(alt_ssid));
        Serial.println("   Password: " + String(alt_password));
        Serial.println("   Waiting 30 seconds for you to set up the hotspot...");
        
        // Give time to set up hotspot
        for (int i = 30; i > 0; i--) {
          Serial.print("⏱️ ");
          Serial.print(i);
          Serial.println(" seconds remaining...");
          delay(1000);
        }
      } else if (networkSelection == 0) {
        // We've tried everything, start printing additional diagnostics
        printAdvancedDiagnostics();
      }
    }
    
    // Start next attempt
    startNextConnectionAttempt();
  }
}

void printWiFiStatus(int status) {
  switch (status) {
    case WL_IDLE_STATUS:
      Serial.println("IDLE");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("NO SSID AVAILABLE (Can't find the WiFi network)");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("SCAN COMPLETED");
      break;
    case WL_CONNECTED:
      Serial.println("CONNECTED");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("CONNECTION FAILED (Password may be incorrect)");
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

void printAdvancedDiagnostics() {
  Serial.println("\n=========== ADVANCED DIAGNOSTICS ===========");
  Serial.println("All connection methods failed on both networks.");
  Serial.println("Here are some things to check:");
  
  Serial.println("\n1. WiFi Settings:");
  Serial.println("   - Verify the SSID and password are correct (case-sensitive)");
  Serial.println("   - Try creating a hotspot with a simple password (only numbers or letters)");
  Serial.println("   - Make sure your WiFi is using 2.4GHz (not 5GHz)");
  Serial.println("   - Set your router to use WPA2 security (not WPA3)");
  
  Serial.println("\n2. ESP32 Hardware:");
  Serial.println("   - Try powering your ESP32 from a different USB port or power supply");
  Serial.println("   - Move the ESP32 closer to the WiFi router");
  Serial.println("   - Try a factory reset of your ESP32 by holding the BOOT button while resetting");
  
  Serial.println("\n3. Router Settings:");
  Serial.println("   - Try setting a static IP in this program by changing 'useStaticIP' to true");
  Serial.println("   - Disable MAC filtering if enabled");
  Serial.println("   - Try creating a guest network with simpler security settings");
  Serial.println("   - Restart your router");
  
  Serial.println("\n4. Try Static IP:");
  Serial.println("   - Change 'useStaticIP' to true at the top of this program");
  Serial.println("   - Update the IP addresses to match your network");
  
  Serial.println("\nThis program will continue trying connections in a loop.");
  Serial.println("===============================================");
}