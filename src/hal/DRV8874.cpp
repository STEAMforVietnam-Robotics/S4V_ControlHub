#include "DRV8874.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

DRV8874::DRV8874() : _pwm(nullptr), _in1(0), _in2(0) {}

void DRV8874::attach(Adafruit_PWMServoDriver* pwm, uint8_t in1_ch, uint8_t in2_ch) {
    _pwm = pwm;
    _in1 = in1_ch;
    _in2 = in2_ch;
    // Start at coast so the motor doesn't twitch on boot.
    stop(STOP_COAST);
}

void DRV8874::_write(uint8_t ch, uint16_t value12) {
    if (!_pwm) return;
    // setPWM(ch, on_time, off_time). For simple duty cycle use setPin().
    if (value12 == 0) {
        _pwm->setPWM(ch, 0, 4096);          // fully off
    } else if (value12 >= 4095) {
        _pwm->setPWM(ch, 4096, 0);          // fully on
    } else {
        _pwm->setPWM(ch, 0, value12);
    }
}

void DRV8874::drive(float pct) {
    if (!_pwm) return;
    if (pct >  100.0f) pct =  100.0f;
    if (pct < -100.0f) pct = -100.0f;

    // Convert magnitude to 12-bit duty (0..4095).
    uint16_t duty = (uint16_t)(fabsf(pct) * 40.95f);

    if (pct > 0.0f) {
        _write(_in1, duty);
        _write(_in2, 0);
    } else if (pct < 0.0f) {
        _write(_in1, 0);
        _write(_in2, duty);
    } else {
        stop(STOP_COAST);
    }
}

void DRV8874::stop(StopMode mode) {
    if (!_pwm) return;
    if (mode == STOP_BRAKE) {
        _write(_in1, 4095);
        _write(_in2, 4095);
    } else {  // STOP_COAST
        _write(_in1, 0);
        _write(_in2, 0);
    }
}
