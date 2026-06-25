#include "Motor.h"
#include "../ch_config.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static Encoder s_enc_m1;
static Encoder s_enc_m2;

MotorManager motor;

MotorManager::MotorManager() {
    for (int i = 0; i < MOTOR_COUNT_; ++i) {
        _ch[i].encoder  = nullptr;
        _ch[i].kp = CH_DEFAULT_KP;
        _ch[i].ki = CH_DEFAULT_KI;
        _ch[i].kd = CH_DEFAULT_KD;
        _ch[i].mode = MODE_OPEN_LOOP;
        _ch[i].setpoint = 0.0f;
        _ch[i].integ = 0.0f;
        _ch[i].prev_err = 0.0f;
        _ch[i].meas_rpm = 0.0f;
        _ch[i].output = 0.0f;
    }
}

void MotorManager::_init(Adafruit_PWMServoDriver* pwm) {
    _ch[0].driver.attach(pwm, CH_M1_IN1_CH, CH_M1_IN2_CH);
    _ch[1].driver.attach(pwm, CH_M2_IN1_CH, CH_M2_IN2_CH);
    _ch[2].driver.attach(pwm, CH_M3_IN1_CH, CH_M3_IN2_CH);
    _ch[3].driver.attach(pwm, CH_M4_IN1_CH, CH_M4_IN2_CH);
    _ch[4].driver.attach(pwm, CH_M5_IN1_CH, CH_M5_IN2_CH);
    _ch[5].driver.attach(pwm, CH_M6_IN1_CH, CH_M6_IN2_CH);

    if (s_enc_m1.attach(CH_M1_ENC_A, CH_M1_ENC_B, CH_M1_CPR))
        _ch[0].encoder = &s_enc_m1;
    if (s_enc_m2.attach(CH_M2_ENC_A, CH_M2_ENC_B, CH_M2_CPR))
        _ch[1].encoder = &s_enc_m2;

    xTaskCreatePinnedToCore(&MotorManager::_taskTrampoline, "ch_pid",
        CH_PID_TASK_STACK, this, CH_PID_TASK_PRIORITY, nullptr, CH_PID_TASK_CORE);
}

void MotorManager::_taskTrampoline(void* arg) {
    static_cast<MotorManager*>(arg)->_taskLoop();
}

void MotorManager::_taskLoop() {
    const TickType_t period = pdMS_TO_TICKS(1000 / CH_PID_TASK_HZ);
    TickType_t last_wake = xTaskGetTickCount();
    const float dt_s = 1.0f / (float)CH_PID_TASK_HZ;
    while (true) {
        _pid_step(dt_s);
        vTaskDelayUntil(&last_wake, period);
    }
}

void MotorManager::_pid_step(float dt_s) {
    for (int i = 0; i < MOTOR_COUNT_; ++i) {
        Channel& c = _ch[i];
        if (c.encoder) c.meas_rpm = c.encoder->updateRpm(dt_s);
        if (c.mode == MODE_OPEN_LOOP) continue;
        if (!c.encoder) { c.output = 0.0f; c.driver.stop(STOP_COAST); continue; }

        float meas, err;
        if (c.mode == MODE_SPEED) {
            meas = c.meas_rpm; err = c.setpoint - meas;
        } else {
            meas = c.encoder->positionDeg(); err = c.setpoint - meas;
        }
        c.integ += err * dt_s;
        const float I_CLAMP = 200.0f;
        if (c.integ >  I_CLAMP) c.integ =  I_CLAMP;
        if (c.integ < -I_CLAMP) c.integ = -I_CLAMP;
        float deriv = (err - c.prev_err) / dt_s;
        c.prev_err = err;
        float u = c.kp * err + c.ki * c.integ + c.kd * deriv;
        if (u >  100.0f) u =  100.0f;
        if (u < -100.0f) u = -100.0f;
        c.output = u;
        c.driver.drive(u);
    }
}

void MotorManager::rotate(MotorId id, float degrees, float speed_pct) {
    int i = _idx(id);
    if (i < 0 || i >= MOTOR_COUNT_) return;
    Channel& c = _ch[i];
    if (!c.encoder) {
        float deg_per_s = 6.0f * speed_pct;
        if (deg_per_s < 1.0f) return;
        float t_s = fabsf(degrees) / deg_per_s;
        c.mode = MODE_OPEN_LOOP;
        c.output = (degrees >= 0 ? speed_pct : -speed_pct);
        c.driver.drive(c.output);
        delay((uint32_t)(t_s * 1000.0f));
        c.driver.stop();
        c.output = 0.0f;
        return;
    }
    float start = c.encoder->positionDeg();
    c.setpoint = start + degrees;
    c.integ = 0.0f;
    c.prev_err = 0.0f;
    c.mode = MODE_POSITION;
    const float TOL_DEG = 2.0f;
    const uint32_t TIMEOUT_MS = (uint32_t)(fabsf(degrees) * 20.0f) + 1000;
    uint32_t t0 = millis();
    while (millis() - t0 < TIMEOUT_MS) {
        if (fabsf(c.setpoint - c.encoder->positionDeg()) < TOL_DEG) break;
        delay(10);
    }
    stop(id, STOP_BRAKE);
}

void MotorManager::stop(MotorId id, StopMode mode) {
    int i = _idx(id);
    if (i < 0 || i >= MOTOR_COUNT_) return;
    _ch[i].mode = MODE_OPEN_LOOP;
    _ch[i].setpoint = 0.0f;
    _ch[i].output = 0.0f;
    _ch[i].driver.stop(mode);
}

void MotorManager::stopAll(StopMode mode) {
    for (int i = 1; i <= MOTOR_COUNT_; ++i) stop((MotorId)i, mode);
}

float MotorManager::position(MotorId id) {
    int i = _idx(id);
    if (i < 0 || i >= MOTOR_COUNT_) return 0.0f;
    return _ch[i].encoder ? _ch[i].encoder->positionDeg() : 0.0f;
}

void MotorManager::set_P(MotorId id, float v) { int i=_idx(id); if (i>=0) _ch[i].kp = v; }
void MotorManager::set_I(MotorId id, float v) { int i=_idx(id); if (i>=0) _ch[i].ki = v; }
void MotorManager::set_D(MotorId id, float v) { int i=_idx(id); if (i>=0) _ch[i].kd = v; }
float MotorManager::get_P(MotorId id) const { int i=_idx(id); return i>=0 ? _ch[i].kp : 0; }
float MotorManager::get_I(MotorId id) const { int i=_idx(id); return i>=0 ? _ch[i].ki : 0; }
float MotorManager::get_D(MotorId id) const { int i=_idx(id); return i>=0 ? _ch[i].kd : 0; }

void MotorManager::set_controlmode(MotorId id, ControlMode m) {
    int i = _idx(id); if (i < 0) return;
    _ch[i].mode = m; _ch[i].integ = 0.0f; _ch[i].prev_err = 0.0f;
}
ControlMode MotorManager::get_controlmode(MotorId id) const {
    int i = _idx(id); return i>=0 ? _ch[i].mode : MODE_OPEN_LOOP;
}

void MotorManager::set_speed(MotorId id, float rpm) {
    int i = _idx(id); if (i < 0) return;
    _ch[i].mode = MODE_SPEED; _ch[i].setpoint = rpm;
}
float MotorManager::get_speed(MotorId id) {
    int i = _idx(id); return i >= 0 ? _ch[i].meas_rpm : 0.0f;
}
void MotorManager::set_position(MotorId id, float deg) {
    int i = _idx(id); if (i < 0) return;
    _ch[i].mode = MODE_POSITION; _ch[i].setpoint = deg;
}
float MotorManager::get_position(MotorId id) { return position(id); }
void MotorManager::drive_raw(MotorId id, float pct) {
    int i = _idx(id); if (i < 0) return;
    _ch[i].mode = MODE_OPEN_LOOP; _ch[i].output = pct; _ch[i].driver.drive(pct);
}
void MotorManager::reset_encoder(MotorId id) {
    int i = _idx(id); if (i < 0 || !_ch[i].encoder) return;
    _ch[i].encoder->reset();
}
bool MotorManager::has_encoder(MotorId id) const {
    int i = _idx(id); return i >= 0 && _ch[i].encoder != nullptr;
}
