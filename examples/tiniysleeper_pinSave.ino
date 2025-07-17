#include <TinySleeper.h>

#define LED_PIN 1      // A general purpose LED
#define MOSFET_PIN 0   // A pin controlling a device that must stay OFF

void setup() {
  wdt_disable();
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);

  // --- Configure TinySleeper for maximum power saving ---
  
  // 1. Enable automatic pin management. The library will now handle
  //    setting pins to a low-power state (INPUT_PULLUP) during sleep.
  TinySleeper.enablePinManagement(true);
  
  // 2. Exclude the MOSFET pin. We need this pin to remain OUTPUT and LOW
  //    to keep the connected device off. The library will not touch it.
  TinySleeper.excludePin(MOSFET_PIN);

  // We are not excluding LED_PIN. The library will save its state (OUTPUT),
  // change it to INPUT_PULLUP during sleep, and restore it to OUTPUT after waking.
}

void loop() {
  // Blink the LED to show that the pin state was correctly restored after sleep.
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);

  // Go to sleep for 10 seconds.
  // During this time, all managed pins are in low-power mode,
  // and the MOSFET_PIN remains safely LOW.
  TinySleeper.sleep(10000);
}