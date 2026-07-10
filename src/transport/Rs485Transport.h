/*  Rs485Transport.h
 *
 *  Half-duplex RS485 transceiver driver. Uses one HardwareSerial plus two
 *  GPIOs (DE, RE) to switch bus direction. During transmit DE=1 & RE=1;
 *  during receive both are pulled low so the hub only listens.
 */

#ifndef CONTROLHUB_RS485_TRANSPORT_H
#define CONTROLHUB_RS485_TRANSPORT_H

#include "Transport.h"
#include "../ControlHubConfig.h"

namespace ControlHubProtocol {

class Rs485Transport : public Transport {
public:
    Rs485Transport(HardwareSerial &serial = CH_RS485_SERIAL,
                   int dePin  = CH_RS485_DE,
                   int rePin  = CH_RS485_RE);

    /** Configure UART pins and baud. rxPin/txPin=-1 keeps board defaults. */
    void begin(unsigned long baud = CH_RS485_BAUD,
               int8_t rxPin = -1, int8_t txPin = -1);
    void end();

    int    available() override;
    int    read()      override;
    size_t write(const uint8_t *data, size_t len) override;
    bool   connected() override { return _started; }
    const char *name() const override { return "RS485"; }

private:
    void _txMode();
    void _rxMode();

    HardwareSerial &_ser;
    int  _dePin, _rePin;
    bool _started = false;
};

} // namespace
#endif
