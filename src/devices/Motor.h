// ============================================================================
//  devices/Motor.h — Motor manager (core hardware, always present)
//
//  Exposes the global instance `motor` which carries both:
//   - Level-2 (Action) API: rotate(), stop(), position(), set_speed(), ...
//   - Level-3 (Device) API: set_P/I/D(), get_P/I/D(), set_controlmode(), ...
//
//  All API methods take a MotorId (1..6) as first argument.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../hal/DRV8874.h"
#include "../hal/Encoder.h"

class Adafruit_PWMServoDriver;

class MotorManager {
public:
    MotorManager();
    void _init(Adafruit_PWMServoDriver* pwm);

    // Level 2
    void  rotate(MotorId id, float degrees, float speed_pct = 50.0f);
    void  stop(MotorId id, StopMode mode = STOP_COAST);
    void  stopAll(StopMode mode = STOP_COAST);
    float position(MotorId id);

    // Level 3
    void  set_P(MotorId id, float kp);
    void  set_I(MotorId id, float ki);
    void  set_D(MotorId id, float kd);
    float get_P(MotorId id) const;
    float get_I(MotorId id) const;
    float get_D(MotorId id) const;
    void        set_controlmode(MotorId id, ControlMode m);
    ControlMode get_controlmode(MotorId id) const;
    void  set_speed(MotorId id, float rpm);
    float get_speed(MotorId id);
    void  set_position(MotorId id, float deg);
    float get_position(MotorId id);
    void  drive_raw(MotorId id, float pct);
    void  reset_encoder(MotorId id);
    bool  has_encoder(MotorId id) const;

    void  _pid_step(float dt_s);

private:
    struct Channel {
        DRV8874  driver;
        Encoder* encoder;
        float kp, ki, kd;
        ControlMode mode;
        float setpoint;
        float integ;
        float prev_err;
        float meas_rpm;
        float output;
    };
    Channel _ch[MOTOR_COUNT_];
    static int _idx(MotorId id) { return (int)id - 1; }
    static void _taskTrampoline(void* arg);
    void        _taskLoop();
};

extern MotorManager motor;
