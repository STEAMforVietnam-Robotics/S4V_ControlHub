/*  Module.h
 *
 *  Common base for every on-board module (Motor, Servo, Ultrasonic, Contact).
 *  ControlHub keeps a single Module* array and routes an incoming frame to the
 *  module whose moduleType() + id() match — no need for one array per kind.
 */

#ifndef CONTROLHUB_MODULE_H
#define CONTROLHUB_MODULE_H

#include <stdint.h>
#include "../protocol/Frame.h"

namespace ControlHubProtocol { class CommHub; }

class Module {
public:
    /** Every constructed module links itself into a global list so ControlHub
     *  can route frames to it — no explicit attach() call needed. */
    Module() : _next(_head) { _head = this; }
    virtual ~Module() = default;

    /** Bring the hardware up. Safe to call more than once (idempotent). */
    virtual void    begin()            = 0;

    /** Wire module type (MOD_MOTOR / MOD_SERVO / MOD_ULTRASONIC / ...). */
    virtual uint8_t moduleType() const = 0;

    /** Address of this module on the wire. */
    virtual uint8_t id()         const = 0;

    /** Handle a frame addressed to this module (control or status request). */
    virtual void    handleFrame(const ControlHubProtocol::Frame &f,
                                ControlHubProtocol::CommHub &hub) = 0;

    // ---- Global registry (intrusive singly-linked list) ------------------
    static Module *head()       { return _head; }
    Module        *next() const { return _next; }

private:
    static Module *_head;   // defined in Module.cpp
    Module        *_next;
};

#endif // CONTROLHUB_MODULE_H
