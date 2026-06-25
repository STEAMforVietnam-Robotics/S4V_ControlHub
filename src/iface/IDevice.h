// ============================================================================
//  iface/IDevice.h — common base for every pluggable device driver.
//
//  Why this exists: on ESP32, C++ global constructors run BEFORE
//  initArduino(), so `auto x = create(...)` at global scope must NOT touch any
//  peripheral (pinMode/Wire/RMT/LEDC) — doing so panics → boot-loop.
//
//  Therefore device construction is split into two phases:
//    1. Constructor  — software only: PortManager::claim() + store config.
//    2. hwInit()      — all peripheral setup. Called from ControlHub.begin()
//                       (or immediately from create() if the hub is already up).
// ============================================================================
#pragma once

class IDevice {
public:
    virtual ~IDevice() = default;

    // Initialise hardware peripherals. Returns true if the device is ready.
    // Must be safe to call exactly once (the registry guards against repeats).
    virtual bool hwInit() = 0;

    // True if the constructor successfully claimed its port (set by drivers
    // that route through _buildI2C, e.g. shared-bus I2C devices).
    bool portClaimed() const { return _claimed; }

protected:
    bool _claimed = false;
};
