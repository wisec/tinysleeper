# TinySleeper

Inspired by Tinysnore, TinySleeper is an advanced, state-aware, and robust low-power sleep library for the ATtiny85/45/25 family of microcontrollers.

This library allows you to put your ATtiny into deep sleep to save battery power, waking it up with the Watchdog Timer. It includes advanced features to automatically manage pin and peripheral states for maximum power efficiency.

## Features

- **State-Aware:** Saves the state of peripherals (ADC, Timers, Analog Comparator) before sleep and restores them upon waking.
- **Robust:** Includes safety checks like `wdt_reset()` to prevent unintended watchdog resets.
- **Advanced Power Saving:** Optional automatic management of all I/O pins, setting them to the lowest power state (INPUT_PULLUP) during sleep.
- **Flexible:** Allows specific pins to be excluded from automatic management.
- **Configurable:** Lets you decide whether to disable the Brown-Out Detector (BOD) during sleep.

## Installation

1.  Download the latest release as a `.zip` file from the repository.
2.  In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...`
3.  Select the downloaded `.zip` file.


## Compatibility

This library is specifically designed for the **AVR ATtiny x5 series** of microcontrollers:

-   ATtiny85
-   ATtiny45
-   ATtiny25

The library will automatically prevent compilation on any other board to avoid errors and undefined behavior.


## API

```
    /**
     * @brief Puts the ATtiny into deep sleep for the specified duration.
     * @param duration_ms Approximate sleep duration in milliseconds.
     */
    void sleep(uint32_t duration_ms);

    /**
     * @brief Enables or disables automatic pin management for maximum power savings.
     * When enabled, it saves the state of all I/O pins, sets them to INPUT_PULLUP,
     * and restores them upon waking up.
     * @param enabled true to enable, false (default) to disable.
     */
    void enablePinManagement(bool enabled);

    /**
     * @brief Excludes a specific pin from automatic management.
     * Useful for pins that must maintain a specific state (e.g., OUTPUT LOW) during sleep.
     * @param pin The pin number (e.g., PB1 or 1) to exclude.
     */
    void excludePin(uint8_t pin);

    /**
     * @brief Determines if the Brown-Out Detector (BOD) should be disabled during sleep.
     * Disabling it saves power but is risky with unstable power supplies.
     * @param enabled true (default) to disable BOD during sleep, false to keep it active.
     */
    void enableBodInSleep(bool disable);

```

## Usage

### Basic Usage

Simply include the library and call the `sleep()` method.

```cpp
#include <TinySleeper.h>

void setup() {
  // Good practice: ensure the WDT is off at startup.
  wdt_disable();
  pinMode(1, OUTPUT); // LED on PB1
}

void loop() {
  // Blink the LED
  digitalWrite(1, HIGH);
  delay(500);
  digitalWrite(1, LOW);

  // Go to sleep for 5 seconds
  TinySleeper.sleep(5000);
  // The ATtiny will wake up here and continue the loop
}
