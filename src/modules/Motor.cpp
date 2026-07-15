#include "Motor.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

// ---------------------------------------------------------------------------
//  Ctor / begin
// ---------------------------------------------------------------------------
Motor::Motor(MotorPort port, bool reverse) : _reverse(reverse)
{
    switch (port) {
        case M1: _ch1 = CH_M1_IN1; _ch2 = CH_M1_IN2; break;
        case M2: _ch1 = CH_M2_IN1; _ch2 = CH_M2_IN2; break;
        case M3: _ch1 = CH_M3_IN1; _ch2 = CH_M3_IN2; break;
        case M4: _ch1 = CH_M4_IN1; _ch2 = CH_M4_IN2; break;
        default: _ch1 = CH_M1_IN1; _ch2 = CH_M1_IN2; break;
    }
    _id = static_cast<uint8_t>(port);       // wire ID == port number (1..4)
}

void Motor::begin()
{
    if (_begun) return;
    _drv.attach(&pca, _ch1, _ch2);          // starts coasting
    _begun = true;
}

// ---------------------------------------------------------------------------
//  Level 2 API
// ---------------------------------------------------------------------------
void Motor::rotate(int16_t speedPercent)
{
    if (speedPercent >  100) speedPercent =  100;
    if (speedPercent < -100) speedPercent = -100;
    if (_reverse) speedPercent = -speedPercent;

    _lastDuty = (int16_t)(speedPercent * 100);   // store as duty x100
    _mode     = MODE_DUTY;
    _drv.drive((float)speedPercent);
}

void Motor::stop(StopMode mode)
{
    _lastDuty = 0;
    _drv.stop(mode);
}

// Signed duty x100 (i.e. -10000..+10000 == -100.00%..+100.00%) from the wire.
void Motor::_writeDuty(int16_t dutyX100)
{
    _lastDuty = dutyX100;
    float pct = (float)dutyX100 / 100.0f;
    if (_reverse) pct = -pct;
    _drv.drive(pct);
}

// ---------------------------------------------------------------------------
//  Frame protocol
// ---------------------------------------------------------------------------
Frame Motor::makeStatusFrame(uint8_t faultCode) const
{
    Frame f;
    f.moduleType = MOD_MOTOR;
    f.moduleId   = _id;
    f.msgId      = MOTOR_MSG_STATUS;
    f.rtr        = 0;
    f.dlc        = 8;
    f.data[0] = (uint8_t)((_lastDuty >> 8) & 0xFF);   // duty upper
    f.data[1] = (uint8_t)(_lastDuty & 0xFF);          // duty lower
    f.data[2] = _mode;                                // current mode
    // Byte3-6 reserved
    f.data[7] = faultCode ? faultCode : _fault;       // fault
    return f;
}

void Motor::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_MOTOR || f.moduleId != _id) return;

    // Status request (RTR=1)?
    if (f.rtr) {
        hub.sendFrame(makeStatusFrame());
        return;
    }

    switch (f.msgId) {
    case MOTOR_MSG_CONTROL: {
        // Byte0 upper, Byte1 lower : duty x100 (signed)
        int16_t duty = (int16_t)(((uint16_t)f.data[0] << 8) | f.data[1]);
        _mode = (f.dlc >= 3) ? f.data[2] : (uint8_t)MODE_DUTY;
        // Only open-loop duty is implemented; SPEED/POSITION fall back to duty
        // so the motor still responds until the PID layer lands.
        _writeDuty(duty);
        break;
    }
    case MOTOR_MSG_CONFIG:
        // Byte0 = new ID. Lets a host rename an on-board motor at runtime.
        if (f.dlc >= 1) _id = f.data[0];
        break;
    default:
        break;   // ignore unknown msg
    }
}
