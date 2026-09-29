/*  Ultrasonic.h
 *
 *  HC-SR04 style ultrasonic distance sensor on a Multi-Function 4P port
 *  (SIG1 = trig, SIG2 = echo). Provides:
 *
 *      L2: get()     -> distance in mm (0 on timeout)
 *          status()  -> non-zero on error
 *
 *  Frame protocol: MOD_ULTRASONIC, MSG_STATUS -> 2-byte distance (mm) + fault.
 */

#ifndef CONTROLHUB_ULTRASONIC_H
#define CONTROLHUB_ULTRASONIC_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../ControlHubTypes.h"
#include "../protocol/Frame.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

class Ultrasonic : public Module {
public:
    /** @param port Multi-function 4P port (P1 or P2). */
    explicit Ultrasonic(Port port, uint8_t id = 0);

    void begin() override;

    /** Return latest measurement in millimetres. 0 if timeout / not ready. */
    uint16_t get();

    /** 0 = OK, non-zero = timeout / out of range. */
    uint8_t status() const { return _fault; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_ULTRASONIC; }
    uint8_t id()         const override { return _id; }

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

    ControlHubProtocol::Frame makeStatusFrame() const;

private:
    Port     _port;
    int      _trig  = -1;
    int      _echo  = -1;
    uint8_t  _id    = 0;
    bool     _begun = false;

    uint16_t _lastMm = 0;
    uint8_t  _fault  = 0;
};

#endif
