#include <Arduino.h>

#ifndef HOLD_BUTTON_H
#define HOLD_BUTTON_H


class HoldButton {
public:
    enum State {
        IDLE,          // ready, waiting for a press
        HOLDING,       // button pressed, counting toward holdTimeMs
        ACTIVE,        // triggered, output on, counting toward onTimeMs
        WAIT_RELEASE   // output off again, waiting for release before re-arming
    };

    HoldButton(const char* name,
               uint8_t buttonPin,
               uint8_t outputPin,
               unsigned long holdTimeMs = 1500,
               unsigned long onTimeMs = 5000,
               bool outputActiveHigh = true);

    void begin();
    void update();

    // Returns true once per successful hold, then clears itself.
    bool wasTriggered();

    bool isPressed() const { return _stablePressed; }
    bool isOutputOn() const { return _outputOn; }
    bool isReady() const { return _state == IDLE; }
    State getState() const { return _state; }
    unsigned long heldTime() const;   // ms the button has been held (0 if not holding)

    void setDebug(bool enabled) { _debug = enabled; }
    void setHoldTime(unsigned long ms) { _holdTimeMs = ms; }
    void setOnTime(unsigned long ms) { _onTimeMs = ms; }

private:
    static const unsigned long DEBOUNCE_MS = 50;
    static const unsigned long PROGRESS_PRINT_MS = 500;

    void setOutput(bool on);
    void log(const __FlashStringHelper* msg);
    void log(const __FlashStringHelper* msg, unsigned long value);

    const char*   _name;
    uint8_t       _buttonPin;
    uint8_t       _outputPin;
    unsigned long _holdTimeMs;
    unsigned long _onTimeMs;
    bool          _activeHigh;

    State         _state;
    bool          _rawPressed;
    bool          _stablePressed;
    bool          _outputOn;
    bool          _triggerFlag;
    bool          _debug;

    unsigned long _lastRawChange;
    unsigned long _pressStart;
    unsigned long _triggerTime;
    unsigned long _lastProgressPrint;
};

#endif
