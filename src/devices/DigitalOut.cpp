#include "DigitalOut.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

DigitalOut::DigitalOut(SensorPort port, bool initial_state)
    : _port(port), _pin(0xFF), _state(initial_state), _initial(initial_state),
      _ok(false)
{
    PortManager::claim(port, DEV_DIGITAL_OUT, CAP_GPIO_DIGITAL);
}

bool DigitalOut::hwInit() {
    _pin = PortManager::pin(_port, 0);
    if (_pin == 0xFF) return false;
    pinMode(_pin, OUTPUT);
    _ok = true;
    write(_initial);            // now drives the pin (write() needs _ok==true)
    return true;
}

DigitalOut::~DigitalOut() { if (_ok) PortManager::release(_port); }

void DigitalOut::on()             { write(true); }
void DigitalOut::off()            { write(false); }
void DigitalOut::toggle()         { write(!_state); }
void DigitalOut::write(bool s)    {
    if (!_ok) return;
    digitalWrite(_pin, s ? HIGH : LOW);
    _state = s;
}

// --- handle proxy ---
static DigitalOut* _asDout(uint8_t slot) {
    IDigitalIO* d = DeviceRegistry::asDigital(slot);
    // We need the concrete class for on/off/etc. The registry stores impl as
    // void*, but the type guard for DEV_DIGITAL_OUT returned IDigitalIO*; we
    // know the underlying is DigitalOut.
    DeviceSlot* s = DeviceRegistry::get(slot);
    if (!s || s->kind != DEV_DIGITAL_OUT) return nullptr;
    (void)d;   // silence unused warning
    return static_cast<DigitalOut*>(s->impl);
}

void DigitalOutHandle::on()              const { DigitalOut* d=_asDout(_slot); if(d) d->on(); }
void DigitalOutHandle::off()             const { DigitalOut* d=_asDout(_slot); if(d) d->off(); }
void DigitalOutHandle::toggle()          const { DigitalOut* d=_asDout(_slot); if(d) d->toggle(); }
void DigitalOutHandle::write(bool s)     const { DigitalOut* d=_asDout(_slot); if(d) d->write(s); }
bool DigitalOutHandle::value()           const { DigitalOut* d=_asDout(_slot); return d ? d->value() : false; }
