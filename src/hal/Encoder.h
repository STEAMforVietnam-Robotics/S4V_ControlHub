// ============================================================================
//  Encoder.h — Quadrature encoder using GPIO interrupts
//
//  Implements x4 decoding (counts every edge of A and B).
//  Only a small number of encoders are expected (2 for this board); we use a
//  static ISR-table dispatch to keep the ISRs tiny and IRAM-resident.
// ============================================================================
#pragma once

#include <stdint.h>
#include <Arduino.h>

class Encoder {
public:
    Encoder();

    // pinA, pinB: GPIO. cpr = counts-per-revolution after quadrature (x4).
    // Returns true on success, false if too many encoders are attached.
    bool attach(uint8_t pinA, uint8_t pinB, int32_t cpr);
    bool isAttached() const { return _slot >= 0; }

    // Raw count (signed). Wraps on int32 overflow only.
    int32_t count() const;

    // Absolute encoder position in degrees (float, can exceed 360).
    float positionDeg() const;

    // Compute speed in RPM since last call. Pass dt in seconds.
    float updateRpm(float dt_s);

    // Reset count to zero (or to a given value).
    void reset(int32_t value = 0);

    // Maximum supported encoders board-wide. Bump if you add more.
    static constexpr uint8_t MAX_ENCODERS = 4;

private:
    int8_t   _slot;       // index into static table, -1 if unattached
    int32_t  _cpr;
    int32_t  _last_count_for_rpm;
};
