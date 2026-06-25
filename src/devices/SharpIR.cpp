#include "SharpIR.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

SharpIR::SharpIR(SensorPort port) : _port(port), _pin(0xFF), _ok(false) {
    PortManager::claim(port, DEV_SHARP_IR, CAP_ADC);
}

bool SharpIR::hwInit() {
    _pin = PortManager::pin(_port, 0);
    if (_pin == 0xFF) return false;
    pinMode(_pin, INPUT);
    _ok = true;
    return true;
}

SharpIR::~SharpIR() { if (_ok) PortManager::release(_port); }

float SharpIR::distance_cm() {
    if (!_ok) return -1.0f;
    int adc = analogRead(_pin);
    float v = adc * (3.3f / 4095.0f);
    if (v < 0.4f) return -1.0f;                      // too far (saturated low)
    // Empirical curve for GP2Y0A21YK: distance ≈ 27.728 * v^-1.2045
    float cm = 27.728f * powf(v, -1.2045f);
    if (cm < 10.0f || cm > 80.0f) return -1.0f;
    return cm;
}

// --- handle proxy ---
float SharpIRHandle::distance_cm() const {
    IDistanceSensor* d = DeviceRegistry::asDistance(_slot);
    return d ? d->distance_cm() : -1.0f;
}
