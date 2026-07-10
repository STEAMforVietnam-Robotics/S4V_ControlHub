#include "UsbTransport.h"

namespace ControlHubProtocol {

UsbTransport::UsbTransport()
    : _ser(CH_USB_SERIAL),
      _begin([](unsigned long b){ CH_USB_SERIAL.begin(b); }),
      _end([]()                 { CH_USB_SERIAL.end();    }) {}

void UsbTransport::begin(unsigned long baud)
{
    if (_started) return;
    _begin(baud);
    _started = true;
}

void UsbTransport::end()
{
    if (!_started) return;
    _end();
    _started = false;
}

int UsbTransport::available()
{
    return _started ? _ser.available() : 0;
}

int UsbTransport::read()
{
    return _started ? _ser.read() : -1;
}

size_t UsbTransport::write(const uint8_t *data, size_t len)
{
    return _started ? _ser.write(data, len) : 0;
}

} // namespace
