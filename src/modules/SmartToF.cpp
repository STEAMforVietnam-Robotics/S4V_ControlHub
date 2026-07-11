#include "SmartToF.h"
#include "../core/CommHub.h"
#include "../ControlHub.h"

using namespace ControlHubProtocol;

SmartToF::SmartToF(uint8_t id) : _id(id) {}

void SmartToF::begin()
{
    // RS485 bus is already started by ControlHub.begin() - nothing to do here.
}

uint16_t SmartToF::get(uint32_t timeoutMs)
{
    Frame reply;
    if (!::ControlHub.comm().requestRs485(MOD_TOF, _id, reply, timeoutMs)) {
        _fault  = 1;   // timeout - no reply from the Smart Module
        _lastMm = 0;
        return 0;
    }
    _lastMm = ((uint16_t)reply.data[0] << 8) | reply.data[1];
    _fault  = reply.data[7];
    return _lastMm;
}

Frame SmartToF::makeStatusFrame() const
{
    Frame f;
    f.moduleType = MOD_TOF;
    f.moduleId   = _id;
    f.msgId      = MSG_STATUS;
    f.dlc        = 8;
    f.data[0]    = (uint8_t)((_lastMm >> 8) & 0xFF);
    f.data[1]    = (uint8_t)(_lastMm & 0xFF);
    f.data[7]    = _fault;
    return f;
}

void SmartToF::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_TOF || f.moduleId != _id) return;
    if (f.rtr) {
        get();                          // round-trip to the Smart Module
        hub.sendFrame(makeStatusFrame());
    }
}
