#include "ToF.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include "../hal/I2CBus.h"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>
#include <Arduino.h>

ToF::ToF(SensorPort port, uint8_t i2c_addr)
    : _port(port), _addr(i2c_addr), _ok(false), _sensor(nullptr)
{
    // Shares the Wire1 bus on a 4P port; several I²C devices may coexist there
    // at distinct addresses.
    _claimed = PortManager::claimI2C(port, DEV_TOF, i2c_addr);
}

bool ToF::hwInit() {
    // Talks over Wire1, brought up lazily on the bound 4P port.
    TwoWire* bus = ch_i2c::bus1();
    if (!bus) {
        Serial.printf("[ControlHub] no I2C1 bus for ToF on %s\n",
                      PortManager::portName(_port));
        return false;
    }
    _sensor = new Adafruit_VL53L0X();
    if (!_sensor->begin(_addr, /*debug=*/false, bus)) {
        Serial.printf("[ControlHub] VL53L0X begin() failed at 0x%02X on %s\n",
                      _addr, PortManager::portName(_port));
        delete _sensor; _sensor = nullptr;
        return false;
    }
    _ok = true;
    return true;
}

ToF::~ToF() {
    if (_sensor) delete _sensor;
    if (_claimed) PortManager::releaseI2C(_port, _addr);
}

float ToF::distance_mm() {
    if (!_ok) return -1.0f;
    VL53L0X_RangingMeasurementData_t m;
    _sensor->rangingTest(&m, false);
    // Status 4 = out of range; treat as -1.
    if (m.RangeStatus == 4) return -1.0f;
    return (float)m.RangeMilliMeter;
}

float ToF::distance_cm() {
    float mm = distance_mm();
    return (mm < 0) ? -1.0f : mm * 0.1f;
}

// --- handle proxies ---
float ToFHandle::distance_cm() const {
    IDistanceSensor* d = DeviceRegistry::asDistance(_slot);
    return d ? d->distance_cm() : -1.0f;
}
float ToFHandle::distance_mm() const {
    DeviceSlot* s = DeviceRegistry::get(_slot);
    if (!s || s->kind != DEV_TOF) return -1.0f;
    return static_cast<ToF*>(s->impl)->distance_mm();
}
