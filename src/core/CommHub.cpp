#include "CommHub.h"

namespace ControlHubProtocol {

CommHub::CommHub() = default;

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
void CommHub::beginUsb(unsigned long baud) { _usb.begin(baud); }

void CommHub::beginBle(const char *name)   { _ble.begin(name); }

void CommHub::beginRs485(unsigned long baud, int8_t rx, int8_t tx)
{
    _rs485.begin(baud, rx, tx);
}

void CommHub::beginRs485_2(unsigned long baud, int8_t rx, int8_t tx)
{
    _rs485_2.begin(baud, rx, tx);
}

// ---------------------------------------------------------------------------
//  Poll & dispatch
// ---------------------------------------------------------------------------
void CommHub::update()
{
    Frame f;

    _usb.poll();
    while (_usb.available() > 0) {
        int b = _usb.read();
        if (b < 0) break;
        if (_usbParser.feed(uint8_t(b), f))
            _dispatch(f, "USB");
    }

    _ble.poll();
    while (_ble.available() > 0) {
        int b = _ble.read();
        if (b < 0) break;
        if (_bleParser.feed(uint8_t(b), f))
            _dispatch(f, "BLE");
    }

    _rs485.poll();
    while (_rs485.available() > 0) {
        int b = _rs485.read();
        if (b < 0) break;
        if (_rs485Parser.feed(uint8_t(b), f))
            _dispatch(f, "RS485");
    }

    _rs485_2.poll();
    while (_rs485_2.available() > 0) {
        int b = _rs485_2.read();
        if (b < 0) break;
        if (_rs485_2Parser.feed(uint8_t(b), f))
            _dispatch(f, "RS485_2");
    }
}

// ---------------------------------------------------------------------------
//  Outgoing
// ---------------------------------------------------------------------------
void CommHub::sendFrame(const Frame &f)
{
    uint8_t buf[CH_FRAME_TOTAL_MAX];
    size_t n = f.serialize(buf);

    if (_usb.connected()) _usb.write(buf, n);
    if (_ble.connected()) _ble.write(buf, n);
}

void CommHub::sendRs485(const Frame &f)
{
    uint8_t buf[CH_FRAME_TOTAL_MAX];
    size_t n = f.serialize(buf);
    _rs485.write(buf, n);
}

void CommHub::sendRs485_2(const Frame &f)
{
    uint8_t buf[CH_FRAME_TOTAL_MAX];
    size_t n = f.serialize(buf);
    _rs485_2.write(buf, n);
}

bool CommHub::requestRs485(uint8_t modType, uint8_t modId,
                           Frame &reply, uint32_t timeoutMs)
{
    // 1. Send a status request (RTR=1)
    Frame req = Frame::makeStatusRequest(modType, modId);
    sendRs485(req);

    // 2. Poll for a matching reply
    uint32_t deadline = millis() + timeoutMs;
    FrameParser localParser;
    while (millis() < deadline) {
        while (_rs485.available() > 0) {
            int b = _rs485.read();
            if (b < 0) break;
            Frame f;
            if (localParser.feed(uint8_t(b), f)) {
                if (f.moduleType == modType && f.moduleId == modId &&
                    f.msgId == MSG_STATUS) {
                    reply = f;
                    return true;
                }
                // hand off unrelated frames to the normal dispatch path
                _dispatch(f, "RS485");
            }
        }
        yield();
    }
    return false;
}

bool CommHub::requestRs485_2(uint8_t modType, uint8_t modId,
                             Frame &reply, uint32_t timeoutMs)
{
    // 1. Send a status request (RTR=1)
    Frame req = Frame::makeStatusRequest(modType, modId);
    sendRs485_2(req);

    // 2. Poll for a matching reply
    uint32_t deadline = millis() + timeoutMs;
    FrameParser localParser;
    while (millis() < deadline) {
        while (_rs485_2.available() > 0) {
            int b = _rs485_2.read();
            if (b < 0) break;
            Frame f;
            if (localParser.feed(uint8_t(b), f)) {
                if (f.moduleType == modType && f.moduleId == modId &&
                    f.msgId == MSG_STATUS) {
                    reply = f;
                    return true;
                }
                // hand off unrelated frames to the normal dispatch path
                _dispatch(f, "RS485_2");
            }
        }
        yield();
    }
    return false;
}

// ---------------------------------------------------------------------------
//  Routing
// ---------------------------------------------------------------------------
void CommHub::onModule(uint8_t moduleType, FrameHandler cb)
{
    if (moduleType < MAX_MODULE_HANDLERS) _moduleCb[moduleType] = cb;
}

void CommHub::_dispatch(const Frame &f, const char *src)
{
    if (f.moduleType < MAX_MODULE_HANDLERS && _moduleCb[f.moduleType]) {
        _moduleCb[f.moduleType](f, src);
    } else if (_globalCb) {
        _globalCb(f, src);
    }
}

} // namespace
