/*  SmartToF.h
 *
 *  Smart ToF distance sensor - lives on the RS485 Smart Module bus, not
 *  wired directly to the hub. Provides:
 *
 *      L2: get()     -> blocking round-trip over RS485, distance in mm
 *                        (0 on timeout / fault)
 *          status()  -> non-zero on error (from the last get() call)
 *
 *  Frame protocol: MOD_TOF, MSG_STATUS -> 2-byte distance (mm) + fault,
 *  same wire layout as the on-board Ultrasonic sensor.
 */

#ifndef CONTROLHUB_SMARTTOF_H
#define CONTROLHUB_SMARTTOF_H

#include <Arduino.h>
#include "../ControlHubTypes.h"
#include "../protocol/Frame.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

class SmartToF : public Module {
public:
    /** @param id  wire ID of the Smart Module on the RS485 bus.
     *  @param bus which RS485 bus the module is on (RS485_1 or RS485_2). */
    explicit SmartToF(uint8_t id = 0, Rs485Bus bus = RS485_1);

    void begin() override;

    /** Blocking: send a status request to the Smart Module over RS485 and
     *  wait up to @p timeoutMs for its reply. Returns distance in mm
     *  (0 on timeout / fault). */
    uint16_t get(uint32_t timeoutMs = 50);

    /** 0 = OK, non-zero = timeout / fault from the last get() call. */
    uint8_t status() const { return _fault; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_TOF; }
    uint8_t id()         const override { return _id; }

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

    ControlHubProtocol::Frame makeStatusFrame() const;

private:
    uint8_t  _id;
    Rs485Bus _bus;

    uint16_t _lastMm = 0;
    uint8_t  _fault  = 0;
};

#endif
