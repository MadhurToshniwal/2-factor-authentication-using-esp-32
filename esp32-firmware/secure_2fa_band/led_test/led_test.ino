/*
 * Basic LED Blink Test for ESP32 ThingzMini
 * 
 * This simple sketch tests the LED on GPIO2
 */

// Try different pins to see which one controls the LED
const int ledPin1 = 2;  // IO2 - Standard pin for built-in LED
const int ledPin2 = 5;  // IO5 - Sometimes used for LEDs
const int ledPin3 = 13; // IO13 - Sometimes used for LEDs
const int ledPin4 = 22; // IO22 - Sometimes used for LEDs

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32 LED Test");
  
  // Initialize all LED pins as outputs
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  
  // Turn all LEDs off to start
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  digitalWrite(ledPin4, LOW);
}

void loop() {
  // Test LED on pin 2 (IO2)
  Serial.println("Testing LED on pin IO2 (GPIO 2)");
  digitalWrite(ledPin1, HIGH);
  delay(1000);
  digitalWrite(ledPin1, LOW);
  delay(1000);
  
  // Test LED on pin 5 (IO5)
  Serial.println("Testing LED on pin IO5 (GPIO 5)");
  digitalWrite(ledPin2, HIGH);
  delay(1000);
  digitalWrite(ledPin2, LOW);
  delay(1000);
  
  // Test LED on pin 13 (IO13)
  Serial.println("Testing LED on pin IO13 (GPIO 13)");
  digitalWrite(ledPin3, HIGH);
  delay(1000);
  digitalWrite(ledPin3, LOW);
  delay(1000);
  
  // Test LED on pin 22 (IO22)
  Serial.println("Testing LED on pin IO22 (GPIO 22)");
  digitalWrite(ledPin4, HIGH);
  delay(1000);
  digitalWrite(ledPin4, LOW);
  delay(1000);
}