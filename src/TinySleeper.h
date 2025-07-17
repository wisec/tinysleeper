#ifndef TINY_SLEEPER_H
#define TINY_SLEEPER_H

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>


#if !(defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__))
  #error "TinySleeper library is only compatible with the ATtiny x5 series (85, 45, 25)."
#endif

// Define the number of I/O pins for the ATtiny85 (PB0-PB5)
#define ATTINY_IO_PINS 6 

class TinySleeper_t {
public:
    TinySleeper_t(); // Constructor

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
private:
    // Private methods for internal logic
    void saveSystemStates();
    void restoreSystemStates();
    void savePinStates();
    void restorePinStates();
    void setPinsToLowPower();
    void setupWdtForWakeup(int prescaler_index);
    void systemGoToSleep();

    // Member variables to store the state
    bool _pinManagementEnabled;
    bool _bodSleepDisable; // 
    uint8_t _excludedPinsMask; // Bitmask for excluded pins
    
    // Pin register states to save and restore
    uint8_t _savedDDRB;
    uint8_t _savedPORTB;
    
    // Peripheral states
    bool _wasAdcEnabled;
    bool _wasTimer1Enabled;
    bool _wasAnalogCompEnabled;
     
};

// Global instance of the library for easy, singleton-like usage (similar to Serial)
extern TinySleeper_t TinySleeper;

#endif // DEEP_SLEEPER_H