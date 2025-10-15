/*
 * ESP32 Multi-Pin LED Test
 * 
 * This program cycles through several common ESP32 LED pins
 * to help identify which one controls the onboard LED on your board.
 */

// Common LED pins to try
const int LED_PINS[] = {2, 5, 13, 15, 22, 23, 33};
const int NUM_PINS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

int currentPinIndex = 0;
unsigned long lastChangeTime = 0;
const unsigned long CHANGE_INTERVAL = 3000; // 3 seconds per pin

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 Multi-Pin LED Test ===");
  Serial.println("This test will cycle through different GPIO pins to identify the LED pin");
  
  // Initialize all pins as outputs
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  
  Serial.println("Starting pin test sequence...");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Change pin every CHANGE_INTERVAL milliseconds
  if (currentTime - lastChangeTime >= CHANGE_INTERVAL) {
    // Turn off the current pin
    digitalWrite(LED_PINS[currentPinIndex], LOW);
    
    // Move to the next pin
    currentPinIndex = (currentPinIndex + 1) % NUM_PINS;
    
    // Turn on the new pin
    digitalWrite(LED_PINS[currentPinIndex], HIGH);
    
    // Print the current pin being tested
    Serial.print("Testing LED on GPIO pin: ");
    Serial.println(LED_PINS[currentPinIndex]);
    
    lastChangeTime = currentTime;
  }
  
  // Blink the current pin to make it more visible
  if ((currentTime % 500) < 250) {
    digitalWrite(LED_PINS[currentPinIndex], HIGH);
  } else {
    digitalWrite(LED_PINS[currentPinIndex], LOW);
  }
}