/*  CommHub.h
 *
 *  The Comm Hub owns the three transports (USB, BLE, RS485), runs a
 *  FrameParser on each incoming stream, and dispatches valid frames either
 *  to a global user callback or to per-module callbacks that the on-board
 *  Motor/Servo/Sensor classes register during begin().
 *
 *  It also provides a symmetric @ref sendFrame() that broadcasts a frame
 *  out over USB *and* BLE, so a status update reaches whichever host is
 *  currently connected. RS485 frames are addressed to smart modules and
 *  are sent via @ref sendRs485().
 */

#ifndef CONTROLHUB_COMMHUB_H
#define CONTROLHUB_COMMHUB_H

#include "../protocol/Frame.h"
#include "../transport/UsbTransport.h"
#include "../transport/BleTransport.h"
#include "../transport/Rs485Transport.h"

#include <functional>

namespace ControlHubProtocol {

using FrameHandler = std::function<void(const Frame&, const char* source)>;

class CommHub {
public:
    CommHub();

    // ---- lifecycle -----------------------------------------------------
    void beginUsb  (unsigned long baud = CH_USB_BAUD);
    void beginBle  (const char *name   = CH_BLE_DEFAULT_NAME);
    void beginRs485(unsigned long baud = CH_RS485_BAUD,
                    int8_t rxPin = -1, int8_t txPin = -1);
    void beginRs485_2(unsigned long baud = CH_RS485_2_BAUD,
                      int8_t rxPin = -1, int8_t txPin = -1);

    /** Poll transports and dispatch any completed frames. Call from loop(). */
    void update();

    // ---- outgoing ------------------------------------------------------
    /** Send a frame to *host* side (USB and BLE, whichever is up). */
    void sendFrame  (const Frame &f);

    /** Send a frame down RS485 bus #1 (to a Smart Module). */
    void sendRs485  (const Frame &f);

    /** Send a frame down RS485 bus #2 (to a Smart Module). */
    void sendRs485_2(const Frame &f);

    /** Send a status request to an RS485 bus #1 module and wait up to
     *  @p timeoutMs for a reply. Returns true if a matching status frame
     *  arrived; @p reply is then filled. */
    bool requestRs485(uint8_t moduleType, uint8_t moduleId,
                      Frame &reply, uint32_t timeoutMs = 50);

    /** Same as requestRs485() but on RS485 bus #2. */
    bool requestRs485_2(uint8_t moduleType, uint8_t moduleId,
                        Frame &reply, uint32_t timeoutMs = 50);

    // ---- routing -------------------------------------------------------
    /** Global catch-all handler - called for every valid frame, in addition
     *  to any per-module handler. */
    void onFrame(FrameHandler cb) { _globalCb = cb; }

    /** Per-module handler; called alongside the global handler for that
     *  module type. */
    void onModule(uint8_t moduleType, FrameHandler cb);

    // ---- accessors -----------------------------------------------------
    UsbTransport   &usb()    { return _usb;   }
    BleTransport   &ble()    { return _ble;   }
    Rs485Transport &rs485()   { return _rs485;   }
    Rs485Transport &rs485_2() { return _rs485_2; }

private:
    void _dispatch(const Frame &f, const char *source);

    UsbTransport   _usb;
    BleTransport   _ble;
    Rs485Transport _rs485;
    Rs485Transport _rs485_2{CH_RS485_2_SERIAL, CH_RS485_2_DE_RE};

    FrameParser    _usbParser;
    FrameParser    _bleParser;
    FrameParser    _rs485Parser;
    FrameParser    _rs485_2Parser;

    FrameHandler   _globalCb;
    // Small handler table - one slot per module type (0..15 covers current spec).
    static constexpr uint8_t MAX_MODULE_HANDLERS = 16;
    FrameHandler   _moduleCb[MAX_MODULE_HANDLERS];
};

} // namespace
#endif
