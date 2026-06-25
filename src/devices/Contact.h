// ============================================================================
//  devices/Contact.h — Digital input (button, limit switch, bumper)
//
//  Active-low by default (sensor pulls to GND when pressed). Pass
//  active_high=true in ctor for the opposite polarity.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDigitalIO.h"

class Contact : public IDigitalIO {
public:
    explicit Contact(SensorPort port, bool active_high = false);
    ~Contact() override;

    bool hwInit() override;
    bool value() override;          // true == pressed/active
    bool pressed() { return value(); }

private:
    SensorPort _port;
    uint8_t    _pin;
    bool       _active_high;
    bool       _ok;
};

class ContactHandle {
public:
    ContactHandle() : _slot(INVALID_SLOT) {}
    explicit ContactHandle(uint8_t slot) : _slot(slot) {}
    bool pressed() const;
    bool value()   const { return pressed(); }
    bool valid()   const { return _slot != INVALID_SLOT; }
private:
    uint8_t _slot;
};
