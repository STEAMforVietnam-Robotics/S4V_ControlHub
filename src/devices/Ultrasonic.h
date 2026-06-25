// ============================================================================
//  devices/Ultrasonic.h — HC-SR04 driver (4P port only)
//
//  Wiring (on a 4P port):  SIG1 = TRIG, SIG2 = ECHO, VCC, GND.
//  Hardware needs 5V echo level divided to 3.3V for ESP32 (built into board).
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDistanceSensor.h"

class Ultrasonic : public IDistanceSensor {
public:
    explicit Ultrasonic(SensorPort port);
    ~Ultrasonic() override;

    // IDevice
    bool hwInit() override;

    // IDistanceSensor
    float distance_cm() override;

    // Echo-pulse timeout in microseconds (default 30 ms ≈ 5 m max).
    void  set_timeout_us(unsigned long us) { _timeout_us = us; }

private:
    SensorPort    _port;
    uint8_t       _trig;
    uint8_t       _echo;
    bool          _ok;
    unsigned long _timeout_us;
};

// ---- Lightweight handle returned by create(ULTRASONIC, ...) ----
//
// The "real" Ultrasonic object is owned by DeviceRegistry. The handle is just
// a 1-byte slot id; it's copy-able and cheap.
class UltrasonicHandle {
public:
    UltrasonicHandle() : _slot(INVALID_SLOT) {}
    explicit UltrasonicHandle(uint8_t slot) : _slot(slot) {}

    float distance_cm() const;
    bool  valid() const { return _slot != INVALID_SLOT; }

    // Implicit convert to generic DistanceHandle so it can be passed to
    // robot.set_primary_distance() or stored in a polymorphic container.
    operator DistanceHandle() const { return DistanceHandle(_slot); }

private:
    uint8_t _slot;
};
