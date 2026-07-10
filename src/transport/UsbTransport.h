/*  UsbTransport.h  --  Transport over the ESP32 USB CDC (or UART bridge). */
#ifndef CONTROLHUB_USB_TRANSPORT_H
#define CONTROLHUB_USB_TRANSPORT_H

#include <functional>
#include "Transport.h"
#include "../ControlHubConfig.h"

namespace ControlHubProtocol {

class UsbTransport : public Transport {
public:
    /** Bind to the default USB serial (CH_USB_SERIAL / Serial). */
    UsbTransport();

    /** Bind to any serial-like object (HardwareSerial, HWCDC, USBCDC, ...).
     *  All expose Stream I/O plus begin(baud)/end(). */
    template <class T>
    explicit UsbTransport(T &serial)
        : _ser(serial),
          _begin([&serial](unsigned long b){ serial.begin(b); }),
          _end([&serial]()                 { serial.end();    }) {}

    void begin(unsigned long baud = CH_USB_BAUD);
    void end();

    int    available() override;
    int    read()      override;
    size_t write(const uint8_t *data, size_t len) override;
    bool   connected() override { return _started; }
    const char *name() const override { return "USB"; }

private:
    Stream &_ser;
    std::function<void(unsigned long)> _begin;
    std::function<void()>              _end;
    bool _started = false;
};

} // namespace
#endif
