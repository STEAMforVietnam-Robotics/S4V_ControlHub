/*  Transport.h
 *
 *  Abstract byte-stream transport. USB Serial, BLE and RS485 all wrap into
 *  this common interface so CommHub can treat them uniformly.
 */

#ifndef CONTROLHUB_TRANSPORT_H
#define CONTROLHUB_TRANSPORT_H

#include <Arduino.h>

namespace ControlHubProtocol {

class Transport {
public:
    virtual ~Transport() = default;

    /** Bytes waiting to be read. */
    virtual int  available()               = 0;

    /** Read one byte, or -1 if none. */
    virtual int  read()                    = 0;

    /** Blocking write. Returns bytes actually written. */
    virtual size_t write(const uint8_t *data, size_t len) = 0;

    /** True while the underlying link is up (BLE central connected, etc.). */
    virtual bool connected()               { return true; }

    /** Called periodically from CommHub::update() for polling transports. */
    virtual void poll()                    {}

    /** Human-readable name used in debug logs. */
    virtual const char *name() const       = 0;
};

} // namespace

#endif
