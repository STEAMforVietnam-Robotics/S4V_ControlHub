#pragma once

#include "IDevice.h"

class IAnalogSensor : public IDevice {
public:
    virtual ~IAnalogSensor() = default;
    // Raw ADC reading (0..4095 on ESP32 12-bit ADC).
    virtual int   raw()    = 0;
    // Voltage on the pin (0..3.3 V).
    virtual float volts()  = 0;
};
