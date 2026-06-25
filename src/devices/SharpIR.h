// ============================================================================
//  devices/SharpIR.h — Sharp GP2Y0A21 (10–80 cm) analog distance sensor.
//
//  Reads ADC, converts voltage to centimetres via the standard inverse-power
//  curve. Use on a 3P port that supports ADC.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDistanceSensor.h"

class SharpIR : public IDistanceSensor {
public:
    explicit SharpIR(SensorPort port);
    ~SharpIR() override;

    bool hwInit() override;
    float distance_cm() override;

private:
    SensorPort _port;
    uint8_t    _pin;
    bool       _ok;
};

class SharpIRHandle {
public:
    SharpIRHandle() : _slot(INVALID_SLOT) {}
    explicit SharpIRHandle(uint8_t slot) : _slot(slot) {}
    float distance_cm() const;
    bool  valid() const { return _slot != INVALID_SLOT; }
    operator DistanceHandle() const { return DistanceHandle(_slot); }
private:
    uint8_t _slot;
};
