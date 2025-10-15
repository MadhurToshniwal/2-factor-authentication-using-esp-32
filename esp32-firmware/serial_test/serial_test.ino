// Simple ESP32 Serial Test
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32 Serial Test ===");
  Serial.println("If you see this, serial communication is working!");
  Serial.print("ESP32 Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
}

void loop() {
  Serial.println("Hello from ESP32! Time: " + String(millis()));
  delay(2000);
}