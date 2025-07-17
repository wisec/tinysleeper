#include "TinySleeper.h"

#ifndef clear_bit
#define clear_bit(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef set_bit
#define set_bit(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define power_analog_comp_disable() set_bit(ACSR, ACD)

#define power_analog_comp_enable() clear_bit(ACSR, ACD)

TinySleeper_t::TinySleeper_t()
{
    _pinManagementEnabled = false; // By default, pin management is disabled for safety
    _excludedPinsMask = 0;
    _bodSleepDisable = true; // Default: disable BOD during sleep for max power saving
}

void TinySleeper_t::enablePinManagement(bool enabled)
{
    _pinManagementEnabled = enabled;
}

void TinySleeper_t::excludePin(uint8_t pin)
{
    if (pin < ATTINY_IO_PINS)
    {
        _excludedPinsMask |= (1 << pin); // Set the corresponding bit in the mask
    }
}

void TinySleeper_t::enableBodInSleep(bool disable)
{
    _bodSleepDisable = disable;
}

void TinySleeper_t::savePinStates()
{
    // Save the state of the Data Direction (DDRB) and Data (PORTB) registers.
    // This captures the state of all pins (INPUT/OUTPUT/PULLUP/HIGH/LOW) at once.
    _savedDDRB = DDRB;
    _savedPORTB = PORTB;
}

void TinySleeper_t::restorePinStates()
{
    // Restore the register states to what they were before sleeping.
    DDRB = _savedDDRB;
    PORTB = _savedPORTB;
}

void TinySleeper_t::setPinsToLowPower()
{
    // Set all non-excluded pins to INPUT with active PULLUP.
    // This is the lowest-power configuration as it prevents floating pins.

    // 1. Apply the exclusion mask to a copy of the DDRB register
    //    to keep the excluded pins as OUTPUT if they were.
    uint8_t newDDRB = DDRB & _excludedPinsMask;

    // 2. Create the new PORTB configuration:
    //    - Keep the state of the excluded pins.
    //    - Set PULLUPs (bit to 1) for all other pins (which are now INPUTs).
    uint8_t newPORTB = (PORTB & _excludedPinsMask) | (~_excludedPinsMask);

    // Apply the new low-power configurations
    DDRB = newDDRB;
    PORTB = newPORTB;
}

void TinySleeper_t::saveSystemStates()
{
    // Check if the ADC is enabled by reading the ADC Enable bit (ADEN) in its control register.
    _wasAdcEnabled = bit_is_set(ADCSRA, ADEN);

    // Check if Timer1 is enabled. The Power Reduction Register (PRR) holds this info.
    // The timer is enabled if its power reduction bit (PRTIM1) is CLEAR (0).
    _wasTimer1Enabled = bit_is_clear(PRR, PRTIM1);

    // Check if the Analog Comparator is enabled (ACD bit is 0)
    _wasAnalogCompEnabled = bit_is_clear(ACSR, ACD);
}

void TinySleeper_t::restoreSystemStates()
{
    // Only re-enable peripherals if they were enabled before we went to sleep.
    if (_wasAdcEnabled)
        power_adc_enable();

    if (_wasTimer1Enabled)
        power_timer1_enable();

    if (_wasAnalogCompEnabled)
        power_analog_comp_enable();
}

void TinySleeper_t::sleep(uint32_t duration_ms)
{
    // --- PRE-SLEEP PREPARATION ---
    // Save the current state of peripherals and pins
    saveSystemStates();
    if (_pinManagementEnabled)
    {
        savePinStates();
        setPinsToLowPower();
    }

    // Unconditionally disable peripherals for maximum power savings during sleep
    power_adc_disable();
    power_timer1_disable();      // ATtiny85 has Timer0 and Timer1
    power_analog_comp_disable(); // Disable comparator

    // Set the deepest sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // --- CRITICAL SLEEP SEQUENCE ---
    cli(); // Disable global interrupts

    // Reset the WDT to prevent resets from pre-existing short timeouts
    wdt_reset();

    sleep_enable();

    if (_bodSleepDisable)
        sleep_bod_disable();

    sei(); // Re-enable global interrupts (required for WDT wakeup)

    // --- SLEEP CYCLE LOOP ---
    // The WDT's max timeout is 8s. For longer durations, we sleep in cycles.
    while (duration_ms > 0)
    {
        int prescaler = -1;
        // Select the most suitable WDT prescaler
        if (duration_ms >= 8000)
        {
            prescaler = 9;
            duration_ms -= 8000;
        }
        else if (duration_ms >= 4000)
        {
            prescaler = 8;
            duration_ms -= 4000;
        }
        else if (duration_ms >= 2000)
        {
            prescaler = 7;
            duration_ms -= 2000;
        }
        else if (duration_ms >= 1000)
        {
            prescaler = 6;
            duration_ms -= 1000;
        }
        else if (duration_ms >= 500)
        {
            prescaler = 5;
            duration_ms -= 500;
        }
        else if (duration_ms >= 250)
        {
            prescaler = 4;
            duration_ms -= 250;
        }
        else if (duration_ms >= 128)
        {
            prescaler = 3;
            duration_ms -= 128;
        }
        else if (duration_ms >= 64)
        {
            prescaler = 2;
            duration_ms -= 64;
        }
        else if (duration_ms >= 32)
        {
            prescaler = 1;
            duration_ms -= 32;
        }
        else if (duration_ms >= 16)
        {
            prescaler = 0;
            duration_ms -= 16;
        }

        if (prescaler != -1)
        {
            setupWdtForWakeup(prescaler);
            systemGoToSleep();
        }
        else
        {
            // For remaining times shorter than the WDT's minimum, use a standard delay
            delay(duration_ms);
            duration_ms = 0;
        }
    }

    // --- POST-WAKEUP RESTORATION ---
    restoreSystemStates();

    if (_pinManagementEnabled)
        restorePinStates();
}

void TinySleeper_t::systemGoToSleep()
{
    sleep_mode();    // Put the microcontroller to sleep... Zzz...
    sleep_disable(); // Upon wakeup, execution resumes here. Immediately disable sleep mode.
}

void TinySleeper_t::setupWdtForWakeup(int prescaler_index)
{
    // This maps the prescaler index to the WDP bits in the WDTCR register.
    byte wdt_prescaler_bits = prescaler_index & 7; // Bits WDP0-2
    if (prescaler_index > 7)
        wdt_prescaler_bits |= (1 << WDP3); // Bit WDP3

    // Set the WDT to trigger an INTERRUPT, not a RESET.
    wdt_prescaler_bits |= (1 << WDIE);

    // This is the timed sequence required to change WDT settings.
    WDTCR |= (1 << WDCE) | (1 << WDE); // Enable Watchdog Change
    WDTCR = wdt_prescaler_bits;        // Apply new settings
}

// Watchdog Interrupt Service Routine (ISR).
// Its only purpose is to wake the processor.
// The WDT is automatically disabled by the Arduino core upon waking from an ISR,
// but disabling it explicitly here is a good safety practice.
ISR(WDT_vect)
{
    wdt_disable();
}

// Create the global instance of the library
TinySleeper_t TinySleeper;