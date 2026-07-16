#include "Led.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

Led::Led(Port port, uint16_t count, uint8_t id)
    : _port(port), _count(count), _id(id),
      _px(count, 0, NEO_GRB + NEO_KHZ800)
{
    switch (_port) {
        case PORT_MF4P_1: _pin = CH_MF4P_1_SIG1; break;
        case PORT_MF4P_2: _pin = CH_MF4P_2_SIG1; break;
        case PORT_MF3P_1: _pin = CH_MF3P_1_SIG;  break;
        case PORT_MF3P_2: _pin = CH_MF3P_2_SIG;  break;
        case PORT_MF3P_3: _pin = CH_MF3P_3_SIG;  break;
        default:          _pin = -1;             break;
    }
}

void Led::begin()
{
    if (_begun) return;
    if (_pin < 0) { _fault = 1; return; }

    _px.setPin(_pin);
    _px.updateLength(_count);
    _px.begin();
    _px.setBrightness(_brightness);
    _px.clear();
    _px.show();
    _begun = true;
}

void Led::fill(uint8_t r, uint8_t g, uint8_t b)
{
    if (!_begun) begin();
    _px.fill(_px.Color(r, g, b));
}

void Led::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    fill(r, g, b);
    show();
}

void Led::setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
    if (!_begun) begin();
    if (i >= _count) return;
    _px.setPixelColor(i, _px.Color(r, g, b));
    show();
}

void Led::setBrightness(uint8_t b)
{
    _brightness = b;
    if (!_begun) begin();
    _px.setBrightness(b);
    show();
}

void Led::clear()
{
    if (!_begun) begin();
    _px.clear();
    show();
}

void Led::show()
{
    if (!_begun) begin();
    _px.show();
}

Frame Led::makeStatusFrame() const
{
    Frame f;
    f.moduleType = MOD_LED;
    f.moduleId   = _id;
    f.msgId      = MSG_STATUS;
    f.dlc        = 4;
    f.data[0]    = (uint8_t)((_count >> 8) & 0xFF);
    f.data[1]    = (uint8_t)(_count & 0xFF);
    f.data[2]    = _brightness;
    f.data[3]    = _fault;
    return f;
}

void Led::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_LED || f.moduleId != _id) return;

    if (f.rtr) {
        hub.sendFrame(makeStatusFrame());
        return;
    }

    if (f.msgId == MSG_CONTROL && f.dlc >= 4) {
        // data = [index, r, g, b, (brightness)]
        uint8_t index = f.data[0];
        uint8_t r = f.data[1], g = f.data[2], b = f.data[3];
        if (index == 0xFF) setColor(r, g, b);
        else               setPixel(index, r, g, b);
        if (f.dlc >= 5) setBrightness(f.data[4]);
    }
}
