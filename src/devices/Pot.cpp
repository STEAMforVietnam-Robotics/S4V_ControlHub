#include "Pot.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

Pot::Pot(SensorPort port) : _port(port), _pin(0xFF), _ok(false) {
    PortManager::claim(port, DEV_POT, CAP_ADC);
}

bool Pot::hwInit() {
    _pin = PortManager::pin(_port, 0);
    if (_pin == 0xFF) return false;
    // analogRead on ESP32 doesn't strictly require pinMode but it doesn't hurt.
    pinMode(_pin, INPUT);
    _ok = true;
    return true;
}

Pot::~Pot() { if (_ok) PortManager::release(_port); }

int   Pot::raw()       { return _ok ? analogRead(_pin) : 0; }
float Pot::volts()     { return raw() * (3.3f / 4095.0f); }
float Pot::percent()   { return raw() * (100.0f / 4095.0f); }
float Pot::scaled(float a, float b) { return a + (b - a) * (raw() / 4095.0f); }

// --- handle proxy ---
static Pot* _asPot(uint8_t slot) {
    return static_cast<Pot*>(DeviceRegistry::asAnalog(slot));
}
int   PotHandle::raw()      const { Pot* p = _asPot(_slot); return p ? p->raw() : 0; }
float PotHandle::volts()    const { Pot* p = _asPot(_slot); return p ? p->volts() : 0.0f; }
float PotHandle::percent()  const { Pot* p = _asPot(_slot); return p ? p->percent() : 0.0f; }
float PotHandle::scaled(float a, float b) const {
    Pot* p = _asPot(_slot); return p ? p->scaled(a, b) : 0.0f;
}
