#include "Frame.h"
#include "CRC8.h"

namespace ControlHubProtocol {

// ---------------------------------------------------------------------------
//  Frame::computeCrc / serialize
// ---------------------------------------------------------------------------
uint8_t Frame::computeCrc() const
{
    // The CRC is calculated over every byte before the CRC field itself,
    // i.e. Header .. last data byte. Total = 6 + dlc.
    uint8_t buf[1 + 5 + CH_FRAME_MAX_DATA];
    buf[0] = CH_FRAME_HEADER;
    buf[1] = moduleType;
    buf[2] = moduleId;
    buf[3] = msgId;
    buf[4] = rtr;
    buf[5] = dlc;
    for (uint8_t i = 0; i < dlc && i < CH_FRAME_MAX_DATA; ++i)
        buf[6 + i] = data[i];
    return crc8_smbus(buf, 6 + dlc);
}

size_t Frame::serialize(uint8_t *out) const
{
    out[0] = CH_FRAME_HEADER;
    out[1] = moduleType;
    out[2] = moduleId;
    out[3] = msgId;
    out[4] = rtr;
    out[5] = dlc;
    uint8_t n = (dlc > CH_FRAME_MAX_DATA) ? CH_FRAME_MAX_DATA : dlc;
    for (uint8_t i = 0; i < n; ++i) out[6 + i] = data[i];
    out[6 + n] = computeCrc();
    return 7 + n;
}

// ---------------------------------------------------------------------------
//  Frame factory helpers
// ---------------------------------------------------------------------------
Frame Frame::makeMotorControl(uint8_t id, int16_t duty, MotorMode mode)
{
    Frame f;
    f.moduleType = MOD_MOTOR;
    f.moduleId   = id;
    f.msgId      = MSG_CONTROL;
    f.rtr        = 0;
    f.dlc        = 3;
    f.data[0]    = static_cast<uint8_t>((duty >> 8) & 0xFF);   // upper
    f.data[1]    = static_cast<uint8_t>(duty & 0xFF);          // lower
    f.data[2]    = static_cast<uint8_t>(mode);
    return f;
}

Frame Frame::makeMotorConfig(uint8_t currentId, uint8_t newId)
{
    Frame f;
    f.moduleType = MOD_MOTOR;
    f.moduleId   = currentId;
    f.msgId      = MSG_CONFIG;
    f.rtr        = 0;
    f.dlc        = 1;
    f.data[0]    = newId;
    return f;
}

Frame Frame::makeStatusRequest(uint8_t moduleType, uint8_t moduleId, uint8_t msgId)
{
    Frame f;
    f.moduleType = moduleType;
    f.moduleId   = moduleId;
    f.msgId      = msgId;
    f.rtr        = 1;
    f.dlc        = 0;
    return f;
}

// ---------------------------------------------------------------------------
//  FrameParser
// ---------------------------------------------------------------------------
void FrameParser::reset()
{
    _state = S_HEADER;
    _idx   = 0;
}

bool FrameParser::feed(uint8_t b, Frame &out)
{
    switch (_state) {
    case S_HEADER:
        if (b == CH_FRAME_HEADER) {
            _work = Frame{};
            _state = S_MODTYPE;
        }
        break;

    case S_MODTYPE:
        _work.moduleType = b;
        _state = S_MODID;
        break;

    case S_MODID:
        _work.moduleId = b;
        _state = S_MSGID;
        break;

    case S_MSGID:
        _work.msgId = b;
        _state = S_RTR;
        break;

    case S_RTR:
        _work.rtr = b;
        _state = S_DLC;
        break;

    case S_DLC:
        if (b > CH_FRAME_MAX_DATA) {
            reset();                       // bogus length -> resync
            return false;
        }
        _work.dlc = b;
        _idx = 0;
        _state = (b == 0) ? S_CRC : S_DATA;
        break;

    case S_DATA:
        _work.data[_idx++] = b;
        if (_idx >= _work.dlc) _state = S_CRC;
        break;

    case S_CRC: {
        _work.crc = b;
        uint8_t expect = _work.computeCrc();
        reset();
        if (expect == b) {
            out = _work;
            return true;
        }
        break;
    }
    }
    return false;
}

} // namespace
