// ============================================================================
//  PortManager.h — Ownership and capability tracking for multi-function ports
//
//  Every device driver, in its constructor, must call PortManager::claim().
//  If the port is already owned, claim() returns false and the device handle
//  is left in an invalid state (slot = INVALID_SLOT).
// ============================================================================
#pragma once

#include "ch_types.h"
#include <stdint.h>

// Port capability bitfield. A driver checks these in its ctor; PortManager
// rejects the claim if the requested capabilities aren't available.
enum PortCap : uint16_t {
    CAP_NONE          = 0,
    CAP_GPIO_DIGITAL  = 1 << 0,
    CAP_GPIO_TWO      = 1 << 1,   // two signal lines (4P only)
    CAP_ADC           = 1 << 2,
    CAP_PWM           = 1 << 3,   // LEDC PWM
    CAP_I2C           = 1 << 4,
    CAP_RMT           = 1 << 5,   // for WS2812
};

class PortManager {
public:
    // Claim a port for a device. Returns true on success.
    // `kind` is informational (used in error messages).
    // `required_caps` is bitmask of PortCap flags the driver needs.
    static bool claim(SensorPort port, DeviceKind kind, uint16_t required_caps);

    // Release a port. Only the current owner should call this.
    static void release(SensorPort port);

    // --- Shared I2C (Wire1) bus on a 4P port ---------------------------------
    // I2C devices don't own a port exclusively: at most one MF4P port hosts the
    // Wire1 bus, and several devices may share it at distinct addresses.
    // Returns true if the (port, addr) pair was accepted.
    static bool claimI2C(SensorPort port, DeviceKind kind, uint8_t addr);
    static void releaseI2C(SensorPort port, uint8_t addr);

    // Is a 4P port currently hosting the Wire1 bus, and which one?
    static bool       i2cBound();
    static SensorPort i2cPort();

    // Inspect capabilities of a port (combination of PortCap bits).
    static uint16_t capabilities(SensorPort port);

    // Returns true if requested caps are a subset of port's caps.
    static bool supports(SensorPort port, uint16_t required_caps);

    // Return signal pin. idx=0 → SIG1 (or SIG), idx=1 → SIG2 (4P only).
    // Returns 0xFF if not available.
    static uint8_t pin(SensorPort port, uint8_t idx = 0);

    // What's currently using this port? DEV_NONE if free.
    static DeviceKind owner(SensorPort port);

    // Pretty name for logging.
    static const char* portName(SensorPort port);
    static const char* deviceName(DeviceKind k);
};
