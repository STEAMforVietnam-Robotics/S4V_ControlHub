/*  Gamepad.h
 *
 *  BLE gamepad receiver. A phone app (or a BLE gamepad) connects to the
 *  on-board Nordic UART Service and pushes MOD_GAMEPAD frames carrying the
 *  button bitmask and joystick / trigger values. This module decodes them
 *  into an easy state API for the sketch - it does NOT touch the BLE stack.
 *
 *      Gamepad gp;
 *      void setup(){ ControlHub.begin(); gp.begin(); }
 *      void loop(){
 *          ControlHub.update();
 *          if (gp.connected()) Robot.go_forward(gp.leftY());
 *          if (gp.justPressed(GP_A)) ControlHub.statusOk();
 *      }
 *
 *  Wire format (MOD_GAMEPAD, MSG_CONTROL, dlc = 6):
 *      data[0] = buttons hi (bit 8..13)
 *      data[1] = buttons lo (bit 0..7)
 *      data[2] = leftX   (128 = centre)
 *      data[3] = leftY
 *      data[4] = rightX
 *      data[5] = rightY
 *
 *  All buttons - including L2/R2 - are digital bits in the mask; there are no
 *  separate analog trigger bytes.
 */

#ifndef CONTROLHUB_GAMEPAD_H
#define CONTROLHUB_GAMEPAD_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../ControlHubTypes.h"
#include "../protocol/Frame.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

// Button bit positions within the mask (14 buttons, bits 0..13).
enum GamepadButton : uint8_t {
    GP_A = 0, GP_B, GP_X, GP_Y,
    GP_UP, GP_DOWN, GP_LEFT, GP_RIGHT,
    GP_L1, GP_R1, GP_R2, GP_L2,
    GP_SELECT, GP_START
};

class Gamepad : public Module {
public:
    /** @param id wire ID (1 for the first / only gamepad). */
    explicit Gamepad(uint8_t id = 1);

    void begin() override;

    // ---- Buttons ---------------------------------------------------------
    /** Current level: true while @p btn is held. */
    bool pressed(uint8_t btn) const { return _buttons & (1u << btn); }
    /** Rising edge since the previous received frame. */
    bool justPressed(uint8_t btn) const {
        return (_buttons & (1u << btn)) && !(_prevButtons & (1u << btn));
    }
    /** Falling edge since the previous received frame. */
    bool justReleased(uint8_t btn) const {
        return !(_buttons & (1u << btn)) && (_prevButtons & (1u << btn));
    }

    // Named helpers
    bool a()      const { return pressed(GP_A); }
    bool b()      const { return pressed(GP_B); }
    bool x()      const { return pressed(GP_X); }
    bool y()      const { return pressed(GP_Y); }
    bool l1()     const { return pressed(GP_L1); }
    bool r1()     const { return pressed(GP_R1); }
    bool l2()     const { return pressed(GP_L2); }
    bool r2()     const { return pressed(GP_R2); }
    bool start()  const { return pressed(GP_START); }
    bool select() const { return pressed(GP_SELECT); }
    bool up()     const { return pressed(GP_UP); }
    bool down()   const { return pressed(GP_DOWN); }
    bool left()   const { return pressed(GP_LEFT); }
    bool right()  const { return pressed(GP_RIGHT); }

    // ---- Joysticks / triggers -------------------------------------------
    int leftX()  const { return _axis(_lx); }   // -100..100
    int leftY()  const { return _axis(_ly); }
    int rightX() const { return _axis(_rx); }
    int rightY() const { return _axis(_ry); }

    /** Raw button mask (bits 0..13). */
    uint16_t buttons() const { return _buttons; }

    /** True while frames keep arriving (auto-clears after the timeout). */
    bool connected() const {
        return _everRx && (millis() - _lastMs < CH_GAMEPAD_TIMEOUT_MS);
    }

    /** Called once per received frame. */
    void onUpdate(void (*cb)(Gamepad &)) { _cb = cb; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_GAMEPAD; }
    uint8_t id()         const override { return _id; }

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

    ControlHubProtocol::Frame makeStatusFrame() const;

private:
    static int _axis(uint8_t raw) {
        int v = ((int)raw - 128) * 100 / 127;
        if (v > 100)  v = 100;
        if (v < -100) v = -100;
        if (v > -8 && v < 8) v = 0;      // ~8% deadzone
        return v;
    }

    uint8_t  _id;
    bool     _begun = false;

    uint16_t _buttons     = 0;
    uint16_t _prevButtons = 0;
    uint8_t  _lx = 128, _ly = 128, _rx = 128, _ry = 128;

    uint32_t _lastMs = 0;
    bool     _everRx = false;
    void (*_cb)(Gamepad &) = nullptr;
};

#endif
