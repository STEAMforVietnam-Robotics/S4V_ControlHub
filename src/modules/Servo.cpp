#include "Servo.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

// LEDC configuration: 50 Hz. The ESP32-S3 LEDC timer is only 14 bits wide
// (SOC_LEDC_TIMER_BIT_WIDTH); asking for more makes ledcAttach()/ledcSetup()
// fail outright and the servo stays silent. 14 bits at 50 Hz gives
// 20000 us / 16384 = ~1.22 us per tick, roughly 0.11 deg on a 180 deg servo.
#if defined(SOC_LEDC_TIMER_BIT_WIDTH)
static constexpr uint8_t SERVO_RES_BITS = (SOC_LEDC_TIMER_BIT_WIDTH < 14)
                                        ? SOC_LEDC_TIMER_BIT_WIDTH : 14;
#else
static constexpr uint8_t SERVO_RES_BITS = 14;
#endif
// Duty counts over one full period (LEDC counts 0..2^bits, not 2^bits - 1).
static constexpr uint32_t SERVO_FULL_SCALE = (1UL << SERVO_RES_BITS);
static constexpr float    SERVO_PERIOD_US  = 1000000.0f / (float)CH_SERVO_FREQ;
static constexpr float    US_TO_TICKS      = (float)SERVO_FULL_SCALE / SERVO_PERIOD_US;

Servo::Servo(ServoPort port, uint16_t servoType) : _port(port), _servoType(servoType)
{
    switch (port) {
        case S1: _pin = CH_SV1_PIN; break;
        case S2: _pin = CH_SV2_PIN; break;
        case S3: _pin = CH_SV3_PIN; break;
        case S4: _pin = CH_SV4_PIN; break;
        default: _pin = CH_SV1_PIN; break;
    }
    // ESP32-S3 exposes only 8 LEDC channels (0..7). Reserve channels 4..7
    // for the four servos so user PWM on channels 0..3 doesn't collide.
    _chan = 4 + (int)_port - 1;
    _id   = static_cast<uint8_t>(_port);    // wire ID == port number (1..4)
}

void Servo::begin()
{
    if (_begun) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    // Arduino-ESP32 3.x: pin-based API, channels are auto-allocated
    if (!ledcAttach(_pin, CH_SERVO_FREQ, SERVO_RES_BITS)) return;
#else
    // Arduino-ESP32 2.x: channel-based API
    if (ledcSetup(_chan, CH_SERVO_FREQ, SERVO_RES_BITS) == 0) return;
    ledcAttachPin(_pin, _chan);
#endif
    _begun = true;
    writeMicroseconds(1500);           // center
}

void Servo::writeMicroseconds(uint16_t us)
{
    if (!_begun) begin();
    if (!_begun) return;               // LEDC setup failed - nothing to drive
    if (us < CH_SERVO_MIN_US) us = CH_SERVO_MIN_US;
    if (us > CH_SERVO_MAX_US) us = CH_SERVO_MAX_US;
    _lastUs = us;
    uint32_t ticks = (uint32_t)((float)us * US_TO_TICKS);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(_pin, ticks);
#else
    ledcWrite(_chan, ticks);
#endif
}

void Servo::write(uint8_t angle)
{
    if (angle > _maxAngle) angle = _maxAngle;
    if (angle < _minAngle) angle = _minAngle;
    _lastAngle = angle;
    uint16_t us = CH_SERVO_MIN_US +
                  ((uint32_t)angle * (CH_SERVO_MAX_US - CH_SERVO_MIN_US)) / _servoType;
    writeMicroseconds(us);
}

void Servo::handleFrame(const Frame &f, CommHub &hub)
{
    if (f.moduleType != MOD_SERVO || f.moduleId != _id) return;

    if (f.rtr) {
        // Reply: current angle + pulse
        Frame s;
        s.moduleType = MOD_SERVO;
        s.moduleId   = _id;
        s.msgId      = MSG_STATUS;
        s.dlc        = 3;
        s.data[0] = _lastAngle;
        s.data[1] = (uint8_t)((_lastUs >> 8) & 0xFF);
        s.data[2] = (uint8_t)(_lastUs & 0xFF);
        hub.sendFrame(s);
        return;
    }

    if (f.msgId == MSG_CONTROL && f.dlc >= 1) {
        // Control frame: Byte0 = target angle in degrees (0..180).
        write(f.data[0]);
    }
}
