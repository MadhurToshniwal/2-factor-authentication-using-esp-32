/*
 * ESP32 Direct GPIO LED Test
 * 
 * This program uses direct GPIO register access to test the LED
 * to bypass any potential Arduino API issues.
 */

// ESP32 GPIO register addresses (from ESP32 Technical Reference Manual)
#define GPIO_OUT_W1TS_REG 0x3FF44008  // Register to set GPIO pins high
#define GPIO_OUT_W1TC_REG 0x3FF4400C  // Register to set GPIO pins low
#define GPIO_ENABLE_W1TS_REG 0x3FF44020  // Register to set GPIO pins as outputs
#define GPIO_IN_REG 0x3FF4403C  // Register to read GPIO input values

// Pin definitions
const int BOOT_BUTTON_PIN = 0;  // BOOT button on ESP32 (GPIO0)
const int LED_PIN = 2;          // LED on ESP32 (GPIO2)

// Direct register access functions
void directSetPinOutput(int pin) {
  *((volatile uint32_t*)GPIO_ENABLE_W1TS_REG) = (1 << pin);
}

void directSetPinHigh(int pin) {
  *((volatile uint32_t*)GPIO_OUT_W1TS_REG) = (1 << pin);
}

void directSetPinLow(int pin) {
  *((volatile uint32_t*)GPIO_OUT_W1TC_REG) = (1 << pin);
}

bool directReadPin(int pin) {
  return (*((volatile uint32_t*)GPIO_IN_REG) >> pin) & 1;
}

void setup() {
  // Initialize serial at 115200 baud
  Serial.begin(115200);
  delay(2000); // Wait for serial
  
  Serial.println("\n\n=== ESP32 DIRECT GPIO TEST ===");
  Serial.println("Testing LED with direct register access");
  
  // Set LED pin as output using direct register access
  directSetPinOutput(LED_PIN);
  
  // Flash LED a few times to confirm it works
  for (int i = 0; i < 5; i++) {
    Serial.println("LED ON");
    directSetPinHigh(LED_PIN);
    delay(500);
    Serial.println("LED OFF");
    directSetPinLow(LED_PIN);
    delay(500);
  }
  
  Serial.println("\nNow LED will stay ON for 3 seconds");
  directSetPinHigh(LED_PIN);
  delay(3000);
  directSetPinLow(LED_PIN);
  
  Serial.println("\nStarting alternating pattern (1 second intervals)");
}

void loop() {
  // Alternate LED on/off every second
  static unsigned long lastToggleTime = 0;
  static bool ledOn = false;
  
  if (millis() - lastToggleTime >= 1000) {
    lastToggleTime = millis();
    ledOn = !ledOn;
    
    if (ledOn) {
      Serial.println("LED ON");
      directSetPinHigh(LED_PIN);
    } else {
      Serial.println("LED OFF");
      directSetPinLow(LED_PIN);
    }
  }
  
  // Check if BOOT button is pressed (use Arduino function for simplicity)
  static bool lastButtonState = true;
  bool currentButtonState = digitalRead(BOOT_BUTTON_PIN);
  
  if (currentButtonState != lastButtonState) {
    if (currentButtonState == LOW) {
      Serial.println("\n*** BOOT BUTTON PRESSED ***");
      
      // Flash LED rapidly when button is pressed
      for (int i = 0; i < 10; i++) {
        directSetPinHigh(LED_PIN);
        delay(50);
        directSetPinLow(LED_PIN);
        delay(50);
      }
    } else {
      Serial.println("BOOT button released");
    }
    lastButtonState = currentButtonState;
  }
}