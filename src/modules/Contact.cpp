#include "Contact.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

Contact::Contact(Port port, uint8_t id, bool activeLow)
    : _port(port), _id(id), _activeLow(activeLow) {
    switch (_port) {
        case P1: _pin = CH_MF4P_1_SIG1; break;
        case P2: _pin = CH_MF4P_2_SIG1; break;
        case P3: _pin = CH_MF3P_1_SIG;  break;
        case P4: _pin = CH_MF3P_2_SIG;  break;
        case P5: _pin = CH_MF3P_3_SIG;  break;
        default:          _pin = -1;             break;
    }
}

void Contact::begin()
{
    if (_begun) return;
    pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
    _begun = true;
}

bool Contact::get()
{
    if (!_begun) begin();
    int raw = digitalRead(_pin);
    bool pressed = _activeLow ? (raw == LOW) : (raw == HIGH);
    _lastState = pressed;
    return pressed;
}

Frame Contact::makeStatusFrame() const
{
    Frame f;
    f.moduleType = MOD_BUMPER;
    f.moduleId   = _id;
    f.msgId      = MSG_STATUS;
    f.dlc        = 8;
    f.data[0]    = _lastState ? 1 : 0;   // Byte 0: Button state
    f.data[1]    = 0;                    // Byte 1: Mode (reserved)
    f.data[7]    = _fault;
    return f;
}

void Contact::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_BUMPER || f.moduleId != _id) return;
    if (f.rtr) {
        get();
        hub.sendFrame(makeStatusFrame());
    }
}
