#include "Gamepad.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

Gamepad::Gamepad(uint8_t id) : _id(id) {}

void Gamepad::begin()
{
    _begun = true;
}

Frame Gamepad::makeStatusFrame() const
{
    Frame f;
    f.moduleType = MOD_GAMEPAD;
    f.moduleId   = _id;
    f.msgId      = MSG_STATUS;
    f.dlc        = 6;
    f.data[0]    = (uint8_t)((_buttons >> 8) & 0xFF);
    f.data[1]    = (uint8_t)(_buttons & 0xFF);
    f.data[2]    = _lx;
    f.data[3]    = _ly;
    f.data[4]    = _rx;
    f.data[5]    = _ry;
    return f;
}

void Gamepad::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_GAMEPAD || f.moduleId != _id) return;

    if (f.rtr) {
        hub.sendFrame(makeStatusFrame());
        return;
    }

    if (f.msgId == MSG_CONTROL && f.dlc >= 6) {
        _prevButtons = _buttons;
        _buttons = ((uint16_t)f.data[0] << 8) | f.data[1];
        _lx = f.data[2];
        _ly = f.data[3];
        _rx = f.data[4];
        _ry = f.data[5];

        _lastMs = millis();
        _everRx = true;

        if (_cb) _cb(*this);
    }
}
