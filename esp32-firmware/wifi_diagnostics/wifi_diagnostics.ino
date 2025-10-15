/*
 * ESP32 WiFi Diagnostics Tool
 * 
 * This tool helps diagnose WiFi connection issues with ESP32 devices by:
 * 1. Scanning available networks
 * 2. Testing connection with different methods
 * 3. Providing detailed error reporting
 * 4. Suggesting solutions based on error codes
 */

#include <WiFi.h>

// ===== WIFI CONFIGURATION =====
// Change these to match your network
const char* your_ssid = "madhur";         // Your WiFi network name
const char* your_password = "12345678";   // Your WiFi password

// Optional: define a secondary network if available
const char* alt_ssid = "";      // Secondary network (leave empty if none)
const char* alt_password = "";  // Secondary password (leave empty if none)

// Hardware configuration
const int LED_PIN = 2;          // Built-in LED on most ESP32 boards

// ===== DIAGNOSTICS CONFIGURATION =====
const bool SCAN_NETWORKS = true;     // Set to false to skip network scanning
const bool RUN_WIFI_TESTS = true;    // Set to false to skip connection tests
const int MAX_CONNECTION_ATTEMPTS = 3; // Number of times to try each method
const int CONNECTION_TIMEOUT = 15000;  // Milliseconds to wait for connection
const int SERIAL_BAUD_RATE = 115200;

// ===== GLOBAL VARIABLES =====
bool foundTargetNetwork = false;
int targetNetworkRSSI = 0;
int targetNetworkChannel = 0;

// ===== UTILITY FUNCTIONS =====

// Flash the LED
void flashLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

// Get string description for WiFi status codes
String getWiFiStatusString(int status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "IDLE";
    case WL_NO_SSID_AVAIL:
      return "NO SSID AVAILABLE - Network not found";
    case WL_SCAN_COMPLETED:
      return "SCAN COMPLETED";
    case WL_CONNECTED:
      return "CONNECTED";
    case WL_CONNECT_FAILED:
      return "CONNECTION FAILED - Password may be incorrect";
    case WL_CONNECTION_LOST:
      return "CONNECTION LOST";
    case WL_DISCONNECTED:
      return "DISCONNECTED";
    default:
      return "UNKNOWN STATUS (" + String(status) + ")";
  }
}

// Get troubleshooting suggestions based on WiFi status
String getTroubleshootingSuggestions(int status) {
  switch (status) {
    case WL_NO_SSID_AVAIL:
      return "- Verify the SSID name (check spelling and case)\n"
             "- Make sure the router is powered on and broadcasting\n"
             "- Try moving closer to the router\n"
             "- Check if your router uses hidden SSID";
    
    case WL_CONNECT_FAILED:
      return "- Double-check your WiFi password\n"
             "- Ensure the router hasn't blocked your device's MAC address\n"
             "- Check if your router uses MAC filtering\n"
             "- Try rebooting your router";
    
    case WL_DISCONNECTED:
      return "- Try moving closer to the router\n"
             "- Ensure router is not experiencing issues\n"
             "- Check if the network is congested\n"
             "- Verify router firmware is up to date";
    
    case WL_CONNECTION_LOST:
      return "- Check signal strength and move closer to router\n"
             "- Ensure stable power supply to the ESP32\n"
             "- Check for interference from other devices";
    
    default:
      return "- Try rebooting both the ESP32 and your router\n"
             "- Check router settings (channel, security type, etc.)\n"
             "- Test with a different WiFi network if possible";
  }
}

// ===== SCANNING FUNCTIONS =====

// Scan for WiFi networks and display detailed information
void scanNetworks() {
  Serial.println("\n====== SCANNING FOR WIFI NETWORKS ======");
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.println("Starting network scan...");
  int networksFound = WiFi.scanNetworks();
  
  if (networksFound == 0) {
    Serial.println("❌ No networks found! Please check:");
    Serial.println("  - Is your router powered on?");
    Serial.println("  - Are you in range of any WiFi networks?");
    Serial.println("  - Is there interference blocking WiFi signals?");
    return;
  }
  
  Serial.print("✅ ");
  Serial.print(networksFound);
  Serial.println(" networks found:");
  Serial.println();
  Serial.println("Signal  Ch  Security   BSSID              Network Name");
  Serial.println("------  --  --------   -----------------  --------------");
  
  // Sort networks by signal strength
  int indices[networksFound];
  for (int i = 0; i < networksFound; i++) {
    indices[i] = i;
  }
  
  // Bubble sort networks by RSSI (signal strength)
  for (int i = 0; i < networksFound; i++) {
    for (int j = i + 1; j < networksFound; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        // Swap
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
      }
    }
  }
  
  // Print network details
  for (int i = 0; i < networksFound; i++) {
    int networkIndex = indices[i];
    String ssid = WiFi.SSID(networkIndex);
    int rssi = WiFi.RSSI(networkIndex);
    String encryption = getEncryptionType(WiFi.encryptionType(networkIndex));
    String bssid = WiFi.BSSIDstr(networkIndex);
    int channel = WiFi.channel(networkIndex);
    
    // Format signal strength indicator
    String signalStr;
    if (rssi >= -50) {
      signalStr = "Excl " + String(rssi);
    } else if (rssi >= -60) {
      signalStr = "Good " + String(rssi);
    } else if (rssi >= -70) {
      signalStr = "Fair " + String(rssi);
    } else {
      signalStr = "Weak " + String(rssi);
    }
    
    // Format output
    char line[128];
    snprintf(line, sizeof(line), "%-7s %2d  %-9s  %-18s %s%s",
             signalStr.c_str(), channel, encryption.c_str(), bssid.c_str(), 
             ssid.c_str(), (ssid == your_ssid) ? " ← TARGET" : "");
    
    Serial.println(line);
    
    // Mark if we found the target network
    if (ssid == your_ssid) {
      foundTargetNetwork = true;
      targetNetworkRSSI = rssi;
      targetNetworkChannel = channel;
    }
  }
  
  Serial.println();
  
  // Report on target network
  if (foundTargetNetwork) {
    Serial.println("✅ Your target network '" + String(your_ssid) + "' was found!");
    Serial.println("   Signal strength: " + String(targetNetworkRSSI) + " dBm");
    Serial.println("   Channel: " + String(targetNetworkChannel));
    
    if (targetNetworkRSSI < -70) {
      Serial.println("⚠️  WARNING: Signal strength is weak. Try moving closer to the router.");
    }
  } else {
    Serial.println("❌ Your target network '" + String(your_ssid) + "' was NOT found!");
    Serial.println("   Please check:");
    Serial.println("   - Network name spelling (SSID is case-sensitive)");
    Serial.println("   - Router is powered on and broadcasting");
    Serial.println("   - ESP32 is within range of the router");
    Serial.println("   - Network is not hidden (if hidden, try connection tests anyway)");
  }
  
  // Clear the scan results
  WiFi.scanDelete();
  
  Serial.println("\n=======================================");
}

// Get encryption type as string
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA+WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-ENT";
    // WPA3 modes removed for compatibility with ESP32 Arduino core 1.0.6
    default:
      return "Unknown";
  }
}

// ===== CONNECTION TESTING FUNCTIONS =====

// Test different WiFi connection methods
void testWiFiConnections() {
  Serial.println("\n====== TESTING WIFI CONNECTION METHODS ======");
  
  const int numMethods = 6;
  bool connectionSuccess = false;
  String methodDescriptions[numMethods] = {
    "Standard connection",
    "Connection with explicit disconnect",
    "Connection with WiFi.persistent(false)",
    "Connection with specific channel",
    "Connection with low power mode disabled",
    "Connection with explicit IP (if configured)"
  };
  
  // If we have a secondary network configured, also try that
  bool hasAltNetwork = (strlen(alt_ssid) > 0 && strlen(alt_password) > 0);
  if (hasAltNetwork) {
    Serial.println("ℹ️ Secondary network configured: " + String(alt_ssid));
  }
  
  // Try each method
  for (int method = 0; method < numMethods; method++) {
    Serial.println("\n----- METHOD " + String(method + 1) + ": " + methodDescriptions[method] + " -----");
    
    // Try with primary network
    connectionSuccess = tryConnection(your_ssid, your_password, method);
    
    // If primary failed and we have a secondary, try that
    if (!connectionSuccess && hasAltNetwork) {
      Serial.println("\nTrying secondary network: " + String(alt_ssid));
      connectionSuccess = tryConnection(alt_ssid, alt_password, method);
    }
    
    // If we connected successfully, break the loop
    if (connectionSuccess) {
      break;
    }
    
    // Disconnect and wait before trying next method
    WiFi.disconnect(true);
    delay(1000);
  }
  
  // Summary
  Serial.println("\n====== CONNECTION TEST SUMMARY ======");
  
  if (connectionSuccess) {
    Serial.println("✅ SUCCESSFULLY CONNECTED TO WIFI!");
    Serial.println("  SSID: " + WiFi.SSID());
    Serial.println("  IP Address: " + WiFi.localIP().toString());
    Serial.println("  Subnet Mask: " + WiFi.subnetMask().toString());
    Serial.println("  Gateway IP: " + WiFi.gatewayIP().toString());
    Serial.println("  DNS Server: " + WiFi.dnsIP().toString());
    Serial.println("  Signal Strength: " + String(WiFi.RSSI()) + " dBm");
    Serial.println("  MAC Address: " + WiFi.macAddress());
    Serial.println("  Channel: " + String(WiFi.channel()));
    
    // Print successful method
    int successMethod = 0;
    for (int i = 0; i < numMethods; i++) {
      if (WiFi.status() == WL_CONNECTED) {
        successMethod = i;
        break;
      }
    }
    
    Serial.println("\nSuccessful method: " + methodDescriptions[successMethod]);
    Serial.println("\nRecommendation: Use this connection method in your main code.");
  } else {
    Serial.println("❌ ALL CONNECTION METHODS FAILED!");
    Serial.println("\nPossible issues:");
    Serial.println("1. Incorrect WiFi credentials (double-check ssid and password)");
    Serial.println("2. Router configuration issues (MAC filtering, IP restrictions)");
    Serial.println("3. ESP32 too far from router (weak signal)");
    Serial.println("4. Router not assigning IP addresses (DHCP issues)");
    Serial.println("5. ESP32 hardware or firmware issues");
    
    Serial.println("\nNext steps:");
    Serial.println("1. Verify your WiFi credentials are correct");
    Serial.println("2. Try connecting to a mobile hotspot");
    Serial.println("3. Try resetting your router");
    Serial.println("4. Move ESP32 closer to router");
    Serial.println("5. Try using static IP configuration");
  }
  
  Serial.println("\n=======================================");
}

// Try a specific connection method
bool tryConnection(const char* ssid, const char* password, int method) {
  bool connected = false;
  int attempts = 0;
  int lastStatus = WL_IDLE_STATUS;
  
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");
  
  while (!connected && attempts < MAX_CONNECTION_ATTEMPTS) {
    attempts++;
    Serial.print("Attempt ");
    Serial.print(attempts);
    Serial.print(" of ");
    Serial.print(MAX_CONNECTION_ATTEMPTS);
    Serial.println("...");
    
    // Always disconnect first
    WiFi.disconnect(true);
    delay(1000);
    
    // Method-specific setup
    switch (method) {
      case 0: // Standard connection
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        break;
        
      case 1: // With explicit disconnect
        WiFi.mode(WIFI_OFF);
        delay(1000);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        break;
        
      case 2: // With persistent disabled
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        break;
        
      case 3: // With specific channel
        if (foundTargetNetwork && targetNetworkChannel > 0) {
          WiFi.mode(WIFI_STA);
          WiFi.begin(ssid, password, targetNetworkChannel);
          Serial.print("Using specific channel: ");
          Serial.println(targetNetworkChannel);
        } else {
          WiFi.mode(WIFI_STA);
          // Try channel 1 as default if we don't know the channel
          WiFi.begin(ssid, password, 1);
          Serial.println("Using default channel 1 (network not found in scan)");
        }
        break;
        
      case 4: // With low power mode disabled
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);
        break;
        
      case 5: // With static IP
        // Note: You need to set these values for your network
        // This is just an example with common home network values
        WiFi.mode(WIFI_STA);
        // Try to get the gateway from the router's typical addresses
        IPAddress staticIP(192, 168, 1, 200);  // Choose an unused IP
        IPAddress gateway(192, 168, 1, 1);    // Common router address
        IPAddress subnet(255, 255, 255, 0);   // Common subnet mask
        IPAddress dns(8, 8, 8, 8);            // Google DNS
        
        if (WiFi.config(staticIP, gateway, subnet, dns)) {
          Serial.println("Static IP configuration set");
          WiFi.begin(ssid, password);
        } else {
          Serial.println("Failed to configure static IP, using DHCP");
          WiFi.begin(ssid, password);
        }
        break;
    }
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    
    // Wait for result or timeout
    Serial.print("Waiting for connection");
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startTime < CONNECTION_TIMEOUT) {
      delay(500);
      Serial.print(".");
      // Flash LED while connecting
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    Serial.println();
    
    // Turn off LED after attempt
    digitalWrite(LED_PIN, LOW);
    
    // Check result
    lastStatus = WiFi.status();
    if (lastStatus == WL_CONNECTED) {
      connected = true;
      Serial.println("✅ CONNECTED SUCCESSFULLY!");
      flashLED(5, 100); // Rapid flash for success
      break;
    } else {
      Serial.print("❌ Connection failed. Status: ");
      Serial.println(getWiFiStatusString(lastStatus));
      
      // Wait before retry
      delay(1000);
    }
  }
  
  // If all attempts failed, provide troubleshooting information
  if (!connected) {
    Serial.println("\n⚠️ All attempts with this method failed!");
    Serial.println("Final status: " + getWiFiStatusString(lastStatus));
    Serial.println("\nTroubleshooting suggestions:");
    Serial.println(getTroubleshootingSuggestions(lastStatus));
  }
  
  return connected;
}

// ===== MAIN FUNCTIONS =====

void setup() {
  // Initialize serial
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Welcome message
  Serial.println("\n\n========================================");
  Serial.println("   ESP32 WIFI DIAGNOSTICS TOOL");
  Serial.println("========================================");
  
  // Print device info
  Serial.println("Device Information:");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());
  
  Serial.println("\nTarget WiFi Network: " + String(your_ssid));
  
  // Flash LED to indicate start
  flashLED(3, 200);
  
  // Scan for networks
  if (SCAN_NETWORKS) {
    scanNetworks();
  }
  
  // Test WiFi connections
  if (RUN_WIFI_TESTS) {
    testWiFiConnections();
  }
  
  Serial.println("\n=== DIAGNOSTICS COMPLETE ===");
  Serial.println("If you still have connection issues, please review the logs above.");
  Serial.println("You can also try the following:");
  Serial.println("1. Check your ESP32 power supply (unstable power can cause WiFi issues)");
  Serial.println("2. Try with a different WiFi network or mobile hotspot");
  Serial.println("3. Try resetting the ESP32 (hold BOOT/IO0 button while resetting)");
  Serial.println("4. Update your router firmware");
  Serial.println("5. Check if your ESP32 board has an external antenna option");
}

void loop() {
  // If we're connected, flash LED occasionally
  if (WiFi.status() == WL_CONNECTED) {
    flashLED(1, 100);
    delay(5000);
  } else {
    // Quick double flash if not connected
    flashLED(2, 100);
    delay(3000);
  }
}