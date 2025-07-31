#include "DeepSleeper.h"

#define LED_PIN 1      // PB1, a general purpose LED
#define MOSFET_PIN 0   // PB0, a pin we want to keep OUTPUT LOW

void setup() {
  // Good practice: ensure the WDT is off at startup.
  wdt_disable();
  
  // Set up our pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW); // Ensure the motor/device is off

  // ---- CONFIGURE DeepSleeper ----
  // 1. Enable advanced pin management for maximum power savings.
  DeepSleeper.enablePinManagement(true);
  
  // 2. Exclude the MOSFET pin from management.
  //    This pin MUST remain OUTPUT and LOW during sleep.
  //    The library will not touch it.
  DeepSleeper.excludePin(MOSFET_PIN);
  
  // Note: We are not excluding LED_PIN. The library will set it to
  // INPUT_PULLUP during sleep and then restore it to OUTPUT after waking.
}

void loop() {
  // Active phase: do something
  Serial.println("Active cycle: Blinking LED.");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  // Sleep phase: go to sleep for 5 seconds
  Serial.println("Going to sleep for 5 seconds...");
  delay(100); // Allow time for the serial monitor to print
  
  DeepSleeper.sleep(5000);
  
  // Upon wakeup, execution resumes here
  Serial.println("Good morning! I've woken up.");
  Serial.println("Pin states have been restored. The LED should work again.");
  
  // The loop will restart, and the LED will blink again, demonstrating
  // that its state was correctly restored.
}