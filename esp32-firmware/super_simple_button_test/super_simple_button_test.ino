/*
 * ESP32 Simple BOOT Button and LED Test
 * 
 * This is a very simple program to test just the BOOT button and LED
 * with no WiFi or other complications.
 */

// Pin definitions
const int BOOT_BUTTON_PIN = 0;  // BOOT button on ESP32 (GPIO0)
const int LED_PIN = 2;          // LED on ESP32 (GPIO2)

// Variables
bool ledState = HIGH;           // Start with LED ON
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;     // Assume button not pressed initially
int buttonState;

void setup() {
  // Initialize serial at 115200 baud
  Serial.begin(115200);
  delay(2000); // Give serial time to connect
  
  // Print some info
  Serial.println("\n\n=== SUPER SIMPLE BOOT BUTTON & LED TEST ===");
  Serial.println("This program tests only the BOOT button and LED");
  Serial.println("Press the BOOT button to toggle the LED");
  
  // Configure pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  
  // Turn LED ON at startup to show it works
  digitalWrite(LED_PIN, ledState);
  Serial.println("LED should be ON now. Press BOOT button to toggle it.");
  
  // Debug info
  Serial.print("BOOT button current state: ");
  Serial.println(digitalRead(BOOT_BUTTON_PIN) == LOW ? "PRESSED" : "NOT PRESSED");
}

void loop() {
  // Check button state with debouncing
  int reading = digitalRead(BOOT_BUTTON_PIN);
  
  // Print button state if changed
  static int lastReading = -1;
  if (reading != lastReading) {
    Serial.print("Button reading: ");
    Serial.println(reading == LOW ? "PRESSED" : "RELEASED");
    lastReading = reading;
  }
  
  // If button state changed, reset debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If button state has been stable for the debounce period
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If button state has changed since last stable reading
    if (reading != buttonState) {
      buttonState = reading;
      
      // If button is pressed (LOW because of INPUT_PULLUP)
      if (buttonState == LOW) {
        // Toggle LED
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        
        // Print LED state
        Serial.print("LED turned ");
        Serial.println(ledState ? "ON" : "OFF");
      }
    }
  }
  
  // Save last button state
  lastButtonState = reading;
  
  // Every 5 seconds, print a reminder
  static unsigned long lastReminderTime = 0;
  if (millis() - lastReminderTime >= 5000) {
    lastReminderTime = millis();
    Serial.print("Waiting for BOOT button press... LED is currently ");
    Serial.println(ledState ? "ON" : "OFF");
    Serial.println("BOOT button is the built-in button on your ESP32 board");
  }
  
  // Blink the LED rapidly if it's ON to make it more visible
  if (ledState) {
    static unsigned long lastBlinkTime = 0;
    static bool blinkState = HIGH;
    
    if (millis() - lastBlinkTime >= 100) {
      lastBlinkTime = millis();
      blinkState = !blinkState;
      digitalWrite(LED_PIN, blinkState);
    }
  }
}