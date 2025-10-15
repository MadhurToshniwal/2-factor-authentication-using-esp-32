// Helper function to print WiFi status in human-readable form
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