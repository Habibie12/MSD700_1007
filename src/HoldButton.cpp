#include "HoldButton.h"

HoldButton::HoldButton(const char* name,
                       uint8_t buttonPin,
                       uint8_t outputPin,
                       unsigned long holdTimeMs,
                       unsigned long onTimeMs,
                       bool outputActiveHigh)
    : _name(name),
      _buttonPin(buttonPin),
      _outputPin(outputPin),
      _holdTimeMs(holdTimeMs),
      _onTimeMs(onTimeMs),
      _activeHigh(outputActiveHigh),
      _state(IDLE),
      _rawPressed(false),
      _stablePressed(false),
      _outputOn(false),
      _triggerFlag(false),
      _debug(true),
      _lastRawChange(0),
      _pressStart(0),
      _triggerTime(0),
      _lastProgressPrint(0) {}

void HoldButton::begin() {
    pinMode(_buttonPin, INPUT_PULLUP);
    pinMode(_outputPin, OUTPUT);
    setOutput(false);

    _rawPressed = (digitalRead(_buttonPin) == LOW);
    _stablePressed = _rawPressed;
    _lastRawChange = millis();
    _state = _stablePressed ? WAIT_RELEASE : IDLE;

    log(_stablePressed ? F("initialized, but pin reads PRESSED -> check wiring")
                       : F("initialized, ready"));
}

void HoldButton::update() {
    unsigned long now = millis();

    // --- Debounce ---
    bool raw = (digitalRead(_buttonPin) == LOW);
    if (raw != _rawPressed) {
        _rawPressed = raw;
        _lastRawChange = now;
    }
    bool pressedEdge = false;
    bool releasedEdge = false;
    if ((now - _lastRawChange) >= DEBOUNCE_MS && _rawPressed != _stablePressed) {
        _stablePressed = _rawPressed;
        pressedEdge = _stablePressed;
        releasedEdge = !_stablePressed;
    }

    // --- State machine ---
    switch (_state) {
    case IDLE:
        if (pressedEdge) {
            _pressStart = now;
            _lastProgressPrint = now;
            _state = HOLDING;
            log(F("pressed, keep holding..."));
        }
        break;

    case HOLDING:
        if (releasedEdge) {
            log(F("released too early (ms):"), now - _pressStart);
            _state = IDLE;
        } else if ((now - _pressStart) >= _holdTimeMs) {
            _triggerFlag = true;
            _triggerTime = now;
            setOutput(true);
            _state = ACTIVE;
            log(F("TRIGGERED -> output ON, you can release"));
        } else if ((now - _lastProgressPrint) >= PROGRESS_PRINT_MS) {
            _lastProgressPrint = now;
            log(F("holding (ms):"), now - _pressStart);
        }
        break;

    case ACTIVE:
        if (releasedEdge) {
            log(F("released, total hold (ms):"), now - _pressStart);
        }
        if ((now - _triggerTime) >= _onTimeMs) {
            setOutput(false);
            if (_stablePressed) {
                _state = WAIT_RELEASE;
                log(F("output OFF, release the button to re-arm"));
            } else {
                _state = IDLE;
                log(F("output OFF, ready for new trigger"));
            }
        }
        break;

    case WAIT_RELEASE:
        if (releasedEdge) {
            _state = IDLE;
            log(F("released, ready for new trigger"));
        }
        break;
    }
}

bool HoldButton::wasTriggered() {
    bool t = _triggerFlag;
    _triggerFlag = false;
    return t;
}

unsigned long HoldButton::heldTime() const {
    return (_state == HOLDING) ? (millis() - _pressStart) : 0;
}

void HoldButton::setOutput(bool on) {
    _outputOn = on;
    digitalWrite(_outputPin, (on == _activeHigh) ? HIGH : LOW);
}

void HoldButton::log(const __FlashStringHelper* msg) {
    if (!_debug) return;
    Serial.print(F("["));
    Serial.print(_name);
    Serial.print(F("] "));
    Serial.println(msg);
}

void HoldButton::log(const __FlashStringHelper* msg, unsigned long value) {
    if (!_debug) return;
    Serial.print(F("["));
    Serial.print(_name);
    Serial.print(F("] "));
    Serial.print(msg);
    Serial.print(F(" "));
    Serial.println(value);
}
