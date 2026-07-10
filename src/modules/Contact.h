/*  Contact.h
 *
 *  Bumper / touch switch. Reads a single GPIO with pull-up; state is
 *  optionally debounced. Exposes the same L2/L3 API layout as the other
 *  sensor modules.
 */

#ifndef CONTROLHUB_CONTACT_H
#define CONTROLHUB_CONTACT_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../protocol/Frame.h"
#include "../ControlHubTypes.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

class Contact : public Module {
public:
    /**
     * @param port      multi-function port the bumper is wired to.
     * @param activeLow true if the switch pulls the line LOW when pressed.
     */
    explicit Contact(Port port, bool activeLow = true);

    void begin() override;

    /** True while the bumper is pressed. */
    bool get();

    /** Latest fault code (0 = OK). */
    uint8_t status() const { return _fault; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_BUMPER; }
    uint8_t id()         const override { return _id; }

    ControlHubProtocol::Frame makeStatusFrame() const;

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

private:
    Port     _port;
    int      _pin;
    bool     _activeLow;
    uint8_t  _id;
    bool     _begun     = false;
    bool     _lastState = false;
    uint8_t  _fault     = 0;
};

#endif
