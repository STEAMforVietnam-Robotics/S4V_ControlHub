// ============================================================================
//  DRV8874.h — Single-motor H-bridge driver driven by PCA9685 PWM channels
//
//  The DRV8874 has two inputs IN1/IN2. We use PH/EN-like dual-PWM control:
//    Forward : IN1 = duty,  IN2 = 0
//    Reverse : IN1 = 0,     IN2 = duty
//    Coast   : IN1 = 0,     IN2 = 0     (both low, high impedance via PWM=0)
//    Brake   : IN1 = 1,     IN2 = 1     (both high → short-brake)
// ============================================================================
#pragma once

#include <stdint.h>
#include "../ControlHubTypes.h"

class Adafruit_PWMServoDriver;   // forward decl

class DRV8874 {
public:
    DRV8874();

    // Bind this motor to two PCA9685 channels.
    void attach(Adafruit_PWMServoDriver* pwm, uint8_t in1_ch, uint8_t in2_ch);

    // Drive at signed PWM in [-100, +100] (percent).
    void drive(float pct);

    // Hard stop, behaviour depends on mode.
    void stop(StopMode mode = STOP_COAST);

    bool isAttached() const { return _pwm != nullptr; }

private:
    Adafruit_PWMServoDriver* _pwm;
    uint8_t  _in1, _in2;

    void _write(uint8_t ch, uint16_t value12);   // 0..4095
};
