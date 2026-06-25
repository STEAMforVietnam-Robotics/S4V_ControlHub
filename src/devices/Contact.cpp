#include "Contact.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

Contact::Contact(SensorPort port, bool active_high)
    : _port(port), _pin(0xFF), _active_high(active_high), _ok(false)
{
    PortManager::claim(port, DEV_CONTACT, CAP_GPIO_DIGITAL);
}

bool Contact::hwInit() {
    _pin = PortManager::pin(_port, 0);
    if (_pin == 0xFF) return false;
    pinMode(_pin, _active_high ? INPUT_PULLDOWN : INPUT_PULLUP);
    _ok = true;
    return true;
}

Contact::~Contact() {
    if (_ok) PortManager::release(_port);
}

bool Contact::value() {
    if (!_ok) return false;
    int v = digitalRead(_pin);
    return _active_high ? (v == HIGH) : (v == LOW);
}

// --- handle proxy ---
bool ContactHandle::pressed() const {
    IDigitalIO* d = DeviceRegistry::asDigital(_slot);
    return d ? d->value() : false;
}
