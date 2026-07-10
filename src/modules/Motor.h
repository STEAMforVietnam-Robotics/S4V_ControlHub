/*  Motor.h
 *
 *  Single on-board DC motor driven by a DRV8874 sitting behind PCA9685.
 *
 *  Levels of API exposed today:
 *      L2  rotate() / stop()
 *
 *  Level-3 closed-loop (speed / position PID on the M1/M2 encoders) is not
 *  wired up yet — see hal/Encoder for the building block. Control frames with
 *  MODE_SPEED / MODE_POSITION are accepted but currently only MODE_DUTY moves
 *  the motor.
 *
 *  The class also handles incoming Motor frames (0x05) forwarded by the
 *  CommHub, so a host on USB/BLE can drive the motor with the exact frame
 *  format defined in the RS485 spec (transparently).
 */

#ifndef CONTROLHUB_MOTOR_H
#define CONTROLHUB_MOTOR_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../ControlHubTypes.h"
#include "../protocol/Frame.h"
#include "../hal/DRV8874.h"
#include "Module.h"
#include <Adafruit_PWMServoDriver.h>

namespace ControlHubProtocol { class CommHub; }

extern Adafruit_PWMServoDriver pca;   // global PCA9685 instance (defined in ControlHub.cpp)

class Motor : public Module {
public:
    /**
     * @param port    selects M1..M6
     * @param reverse if true, the motor's "forward" direction is inverted.
     */
    explicit Motor(MotorPort port, bool reverse = false);

    /** Initialise PCA9685 channels (idempotent - safe to call from setup). */
    void begin() override;

    // ------------------------------------------------------------------
    //  LEVEL 2 API  (Action)
    // ------------------------------------------------------------------
    /** Spin the motor. @p speed is -100..+100 (%). */
    void rotate(int16_t speed);

    /** Stop (coast by default, or short-brake). */
    void stop(StopMode mode = STOP_COAST);

    // ------------------------------------------------------------------
    //  Module interface
    // ------------------------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_MOTOR; }
    uint8_t id()         const override { return _id; }

    void    handleFrame(const ControlHubProtocol::Frame &f,
                        ControlHubProtocol::CommHub &hub) override;

    /** Build a status feedback frame (Msg 0x11) for this motor. */
    ControlHubProtocol::Frame makeStatusFrame(uint8_t faultCode = 0) const;

private:
    void _writeDuty(int16_t dutyX100);      // signed duty x100 -> DRV8874

    uint8_t  _id;
    uint8_t  _ch1, _ch2;                    // PCA9685 channels (IN1, IN2)
    bool     _begun   = false;
    bool     _reverse = false;

    DRV8874  _drv;                          // DRV8874 driver instance

    int16_t  _lastDuty = 0;                 // last commanded duty x100 (signed)
    uint8_t  _mode  = ControlHubProtocol::MODE_DUTY;
    uint8_t  _fault = 0;
};

#endif
