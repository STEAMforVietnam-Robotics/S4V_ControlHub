// ============================================================================
//  devices/DigitalOut.h — Driven digital output (LED, relay, MOSFET gate)
//
//  Implements IDigitalIO::value() to report the current commanded state, so
//  reading back works even though the pin is configured as output.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDigitalIO.h"

class DigitalOut : public IDigitalIO {
public:
    explicit DigitalOut(SensorPort port, bool initial_state = false);
    ~DigitalOut() override;

    bool hwInit() override;
    void on();
    void off();
    void toggle();
    void write(bool state);

    bool value() override { return _state; }

private:
    SensorPort _port;
    uint8_t    _pin;
    bool       _state;
    bool       _initial;        // remembered until hwInit() drives the pin
    bool       _ok;
};

class DigitalOutHandle {
public:
    DigitalOutHandle() : _slot(INVALID_SLOT) {}
    explicit DigitalOutHandle(uint8_t slot) : _slot(slot) {}
    void on()    const;
    void off()   const;
    void toggle() const;
    void write(bool state) const;
    bool value() const;
    bool valid() const { return _slot != INVALID_SLOT; }
private:
    uint8_t _slot;
};
