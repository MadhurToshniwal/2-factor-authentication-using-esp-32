/*
 * ESP32 ThingzMini LED Test
 * 
 * This simple program tests the onboard LED on the ESP32 ThingzMini board.
 * It will cycle through different modes to help identify and confirm LED functionality.
 */

// The LED pin on ThingzMini is GPIO2
const int LED_PIN = 2;

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 ThingzMini LED Test ===");
  Serial.println("This test will cycle through different LED patterns");
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  
  // Turn LED off initially
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("Starting LED test sequence...");
}

void loop() {
  // Test 1: Solid ON for 3 seconds
  Serial.println("\n1. LED ON for 3 seconds");
  digitalWrite(LED_PIN, HIGH);
  delay(3000);
  
  // Off briefly
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  
  // Test 2: Fast blink for 3 seconds
  Serial.println("2. Fast blink for 3 seconds");
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
  delay(1000);
  
  // Test 3: Slow pulse for 5 seconds
  Serial.println("3. Slow pulse for 5 seconds");
  for (int i = 0; i < 5; i++) {
    // Fade up
    for (int brightness = 0; brightness <= 255; brightness += 5) {
      analogWrite(LED_PIN, brightness);
      delay(10);
    }
    // Fade down
    for (int brightness = 255; brightness >= 0; brightness -= 5) {
      analogWrite(LED_PIN, brightness);
      delay(10);
    }
  }
  
  // Off for 2 seconds before restarting the sequence
  digitalWrite(LED_PIN, LOW);
  delay(2000);
  
  Serial.println("\nRestarting test sequence...");
}