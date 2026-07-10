/*  Servo.h
 *
 *  RC servo on one of the four SV1..SV4 headers. Uses the ESP32 LEDC
 *  peripheral so we do not depend on the classic Arduino Servo library.
 */

#ifndef CONTROLHUB_SERVO_H
#define CONTROLHUB_SERVO_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../protocol/Frame.h"
#include "../ControlHubTypes.h"
#include "Module.h"


namespace ControlHubProtocol { class CommHub; }

class Servo : public Module {
public:
    /** @param port selects SV1..SV4 (S1..S4). @param servoType sweep in deg. */
    explicit Servo(ServoPort port, uint16_t servoType = 180);

    void begin() override;

    /** Angle 0..180 deg. */
    void write(uint8_t angle);

    void setLimits(uint16_t minAngle, uint16_t maxAngle) {
        _minAngle = minAngle;
        _maxAngle = maxAngle;
    }

    /** Raw pulse in microseconds (500..2500 typical). */
    void  writeMicroseconds(uint16_t us);

    uint8_t  read() const  { return _lastAngle; }
    uint16_t readMicroseconds() const { return _lastUs; }

    ServoPort port() const { return _port; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_SERVO; }
    uint8_t id()         const override { return _id; }

    /** Frame handler for MOD_SERVO messages. */
    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

private:
    ServoPort _port;
    uint8_t  _id;                   // wire ID == port number (1..4)
    uint8_t  _pin;
    uint8_t  _chan;                 // LEDC channel
    bool     _begun = false;
    uint8_t  _lastAngle = 90;
    uint16_t _lastUs    = 1500;
    uint16_t _minAngle = 0;
    uint16_t _maxAngle = 180;
    uint16_t _servoType;            // 90 or 180 or 270 deg
};

#endif
