#include "DeviceRegistry.h"
#include "iface/IDevice.h"
#include "iface/IDistanceSensor.h"
#include "iface/IDigitalIO.h"
#include "iface/IAnalogSensor.h"
#include "iface/ILightOutput.h"
#include <Arduino.h>

namespace {
    DeviceSlot s_slots[CH_MAX_DEVICES] = {};
}

uint8_t DeviceRegistry::allocate(DeviceKind kind, SensorPort port, IDevice* impl) {
    for (uint8_t i = 0; i < CH_MAX_DEVICES; ++i) {
        if (!s_slots[i].in_use) {
            s_slots[i].in_use = true;
            s_slots[i].inited = false;
            s_slots[i].kind   = kind;
            s_slots[i].port   = port;
            s_slots[i].impl   = impl;
            return i;
        }
    }
    Serial.println("[ControlHub] DeviceRegistry full — increase CH_MAX_DEVICES");
    return INVALID_SLOT;
}

void DeviceRegistry::free(uint8_t slot) {
    if (slot < CH_MAX_DEVICES) {
        s_slots[slot].in_use = false;
        s_slots[slot].inited = false;
        s_slots[slot].impl   = nullptr;
    }
}

void DeviceRegistry::initSlot(uint8_t slot) {
    if (slot >= CH_MAX_DEVICES) return;
    DeviceSlot& s = s_slots[slot];
    if (!s.in_use || s.inited || !s.impl) return;
    s.inited = true;            // mark first so a failed hwInit() isn't retried
    s.impl->hwInit();
}

void DeviceRegistry::initAll() {
    for (uint8_t i = 0; i < CH_MAX_DEVICES; ++i) initSlot(i);
}

DeviceSlot* DeviceRegistry::get(uint8_t slot) {
    if (slot >= CH_MAX_DEVICES || !s_slots[slot].in_use) return nullptr;
    return &s_slots[slot];
}

// Type-guarded accessors. Each kind only matches one interface; mismatches
// return nullptr so handle methods safely become no-ops.
IDistanceSensor* DeviceRegistry::asDistance(uint8_t slot) {
    DeviceSlot* s = get(slot);
    if (!s) return nullptr;
    switch (s->kind) {
        case DEV_ULTRASONIC:
        case DEV_TOF:
        case DEV_SHARP_IR:
            return static_cast<IDistanceSensor*>(s->impl);
        default: return nullptr;
    }
}

IDigitalIO* DeviceRegistry::asDigital(uint8_t slot) {
    DeviceSlot* s = get(slot);
    if (!s) return nullptr;
    switch (s->kind) {
        case DEV_CONTACT:
        case DEV_DIGITAL_OUT:    // DigitalOut also implements digital read (its own state)
            return static_cast<IDigitalIO*>(s->impl);
        default: return nullptr;
    }
}

IAnalogSensor* DeviceRegistry::asAnalog(uint8_t slot) {
    DeviceSlot* s = get(slot);
    if (!s) return nullptr;
    switch (s->kind) {
        case DEV_POT:
            return static_cast<IAnalogSensor*>(s->impl);
        default: return nullptr;
    }
}

ILightOutput* DeviceRegistry::asLight(uint8_t slot) {
    DeviceSlot* s = get(slot);
    if (!s) return nullptr;
    switch (s->kind) {
        case DEV_NEOPIXEL:
            return static_cast<ILightOutput*>(s->impl);
        default: return nullptr;
    }
}
