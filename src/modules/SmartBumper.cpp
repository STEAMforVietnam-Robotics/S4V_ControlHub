#include "SmartBumper.h"
#include "../core/CommHub.h"
#include "../ControlHub.h"

using namespace ControlHubProtocol;

SmartBumper::SmartBumper(uint8_t id, Rs485Bus bus) : _id(id), _bus(bus) {}

void SmartBumper::begin()
{
    // RS485 bus is already started by ControlHub.begin() - nothing to do here.
}

bool SmartBumper::get(uint32_t timeoutMs)
{
    Frame reply;
    bool ok = (_bus == RS485_2)
                  ? ::ControlHub.comm().requestRs485_2(MOD_BUMPER, _id, reply, timeoutMs)
                  : ::ControlHub.comm().requestRs485  (MOD_BUMPER, _id, reply, timeoutMs);
    if (!ok) {
        _fault     = 1;    // timeout - no reply from the Smart Module
        _lastState = false;
        return false;
    }
    _lastState = (reply.data[0] != 0);
    _fault     = reply.data[7];
    return _lastState;
}

Frame SmartBumper::makeStatusFrame() const
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

void SmartBumper::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_BUMPER || f.moduleId != _id) return;
    if (f.rtr) {
        get();                          // round-trip to the Smart Module
        hub.sendFrame(makeStatusFrame());
    }
}
