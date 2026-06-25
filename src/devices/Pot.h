// ============================================================================
//  devices/Pot.h — Rotary or slide potentiometer on a 3P (ADC-capable) port.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IAnalogSensor.h"

class Pot : public IAnalogSensor {
public:
    explicit Pot(SensorPort port);
    ~Pot() override;

    bool  hwInit()   override;
    int   raw()      override;          // 0..4095
    float volts()    override;          // 0..3.3 V

    // Convenience: 0..100 %
    float percent();
    // Mapped to [out_min, out_max]
    float scaled(float out_min, float out_max);

private:
    SensorPort _port;
    uint8_t    _pin;
    bool       _ok;
};

class PotHandle {
public:
    PotHandle() : _slot(INVALID_SLOT) {}
    explicit PotHandle(uint8_t slot) : _slot(slot) {}
    int   raw()     const;
    float volts()   const;
    float percent() const;
    float scaled(float out_min, float out_max) const;
    bool  valid()   const { return _slot != INVALID_SLOT; }
private:
    uint8_t _slot;
};
