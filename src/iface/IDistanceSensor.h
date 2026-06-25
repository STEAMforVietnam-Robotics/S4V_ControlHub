// ============================================================================
//  iface/IDistanceSensor.h
//
//  - IDistanceSensor: pure-virtual interface implemented by every distance
//    driver (Ultrasonic, ToF, SharpIR, ...).
//  - DistanceHandle: type-erased handle. Every specific *Handle (Ultrasonic,
//    ToF, SharpIR) implicitly converts to DistanceHandle, so user code can
//    pass any of them to robot.set_primary_distance().
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "IDevice.h"

class IDistanceSensor : public IDevice {
public:
    virtual ~IDistanceSensor() = default;
    // Distance in centimetres. -1.0f on timeout / out-of-range / not init.
    virtual float distance_cm() = 0;
};

// Generic distance handle — accepts any distance sensor.
class DistanceHandle {
public:
    DistanceHandle() : _slot(INVALID_SLOT) {}
    explicit DistanceHandle(uint8_t slot) : _slot(slot) {}

    float distance_cm() const;
    bool  valid() const { return _slot != INVALID_SLOT; }
    uint8_t slot() const { return _slot; }

private:
    uint8_t _slot;
};
