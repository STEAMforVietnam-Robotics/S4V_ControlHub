#include "Ultrasonic.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

Ultrasonic::Ultrasonic(Port port, uint8_t id) : _port(port), _id(id )
{
}

void Ultrasonic::begin()
{
    if (_begun) return;

    if (_port == P1) {
        _trig = CH_MF4P_1_SIG1;
        _echo = CH_MF4P_1_SIG2;
    } else if (_port == P2) {
        _trig = CH_MF4P_2_SIG1;
        _echo = CH_MF4P_2_SIG2;
    } else {
        // Ultrasonic needs a 4P port (separate trig/echo). Leave uninitialised.
        _begun = false;
        return;
    }

    pinMode(_trig, OUTPUT);
    pinMode(_echo, INPUT);
    digitalWrite(_trig, LOW);
    _begun = true;
}

uint16_t Ultrasonic::get()
{
    if (!_begun) return 0;

    digitalWrite(_trig, LOW);
    delayMicroseconds(2);
    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);

    // 30 ms timeout ~= ~5 m range with headroom
    unsigned long dur = pulseIn(_echo, HIGH, 30000UL);
    if (dur == 0) {
        _fault  = 1;
        _lastMm = 0;
        return 0;
    }
    _fault = 0;
    // mm = us * 0.343 mm/us / 2  (speed of sound / round trip)
    uint16_t mm = (uint16_t)((dur * 343UL) / 2000UL);
    _lastMm = mm;
    return mm;
}

Frame Ultrasonic::makeStatusFrame() const
{
    Frame f;
    f.moduleType = MOD_ULTRASONIC;
    f.moduleId   = _id;
    f.msgId      = MSG_STATUS;
    f.dlc        = 8;
    f.data[0]    = (uint8_t)((_lastMm >> 8) & 0xFF);
    f.data[1]    = (uint8_t)(_lastMm & 0xFF);
    f.data[7]    = _fault;
    return f;
}

void Ultrasonic::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_ULTRASONIC || f.moduleId != _id) return;
    if (f.rtr) {
        get();                          // refresh reading before replying
        hub.sendFrame(makeStatusFrame());
    }
}
