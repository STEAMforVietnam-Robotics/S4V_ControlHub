// ============================================================================
//  DeviceRegistry.h — Internal slot table that owns all created devices.
//
//  Handles (Ultrasonic, Contact, ...) are tiny structs holding only a slot id.
//  They look up the actual device state here.
//
//  Slots are static; max count via CH_MAX_DEVICES in ch_config.h.
// ============================================================================
#pragma once

#include "ch_types.h"
#include <stdint.h>

#ifndef CH_MAX_DEVICES
  #define CH_MAX_DEVICES 16
#endif

// Forward decls — registry stores pointers to the common device base.
class IDevice;
class IDistanceSensor;
class IDigitalIO;
class IAnalogSensor;
class ILightOutput;

struct DeviceSlot {
    bool        in_use;
    bool        inited;        // hwInit() already run for this device
    DeviceKind  kind;
    SensorPort  port;
    IDevice*    impl;          // points to the concrete driver instance
};

class DeviceRegistry {
public:
    // Reserve a slot for a freshly-constructed device. Stores `impl` so handles
    // can look it up. Returns slot id, or INVALID_SLOT on full.
    static uint8_t allocate(DeviceKind kind, SensorPort port, IDevice* impl);

    // Free a slot. Does not delete the impl pointer (the registry never
    // allocated it — the device class is responsible for its own storage).
    static void free(uint8_t slot);

    // Run hwInit() for a single slot exactly once (idempotent via `inited`).
    static void initSlot(uint8_t slot);

    // Run hwInit() for every registered-but-not-yet-initialised device.
    // Called from ControlHub.begin() after Wire/PCA9685 are up.
    static void initAll();

    // Look up a slot. Returns nullptr if invalid or freed.
    static DeviceSlot* get(uint8_t slot);

    // Convenience accessors with type guards.
    static IDistanceSensor* asDistance(uint8_t slot);
    static IDigitalIO*      asDigital(uint8_t slot);
    static IAnalogSensor*   asAnalog(uint8_t slot);
    static ILightOutput*    asLight(uint8_t slot);
};
