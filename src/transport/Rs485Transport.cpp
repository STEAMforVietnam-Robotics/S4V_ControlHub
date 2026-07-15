#include "Rs485Transport.h"

namespace ControlHubProtocol {

Rs485Transport::Rs485Transport(HardwareSerial &serial, int deRePin)
    : _ser(serial), _deRePin(deRePin) {}

void Rs485Transport::begin(unsigned long baud, int8_t rxPin, int8_t txPin)
{
    if (_started) return;

    pinMode(_deRePin, OUTPUT);
    _rxMode();

    if (rxPin >= 0 || txPin >= 0) {
        _ser.begin(baud, SERIAL_8N1, rxPin, txPin);
    } else {
        _ser.begin(baud);
    }
    _started = true;
}

void Rs485Transport::end()
{
    if (!_started) return;
    _ser.end();
    _started = false;
}

void Rs485Transport::_txMode()
{
    digitalWrite(_deRePin, HIGH);   // enable driver, disable receiver
}

void Rs485Transport::_rxMode()
{
    digitalWrite(_deRePin, LOW);    // disable driver, enable receiver
}

int Rs485Transport::available()
{
    return _started ? _ser.available() : 0;
}

int Rs485Transport::read()
{
    return _started ? _ser.read() : -1;
}

size_t Rs485Transport::write(const uint8_t *data, size_t len)
{
    if (!_started) return 0;

    _txMode();
    size_t n = _ser.write(data, len);
    _ser.flush();                 // wait until the shift-register empties
    _rxMode();
    return n;
}

} // namespace
