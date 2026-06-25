#include "Servo.h"
#include "../ch_config.h"
#include <ESP32Servo.h>
#include <Arduino.h>

ServoManager servo;

static const uint8_t kPins[SERVO_COUNT_] = {
    CH_SV1_PIN, CH_SV2_PIN, CH_SV3_PIN, CH_SV4_PIN
};

ServoManager::ServoManager() {
    for (int i = 0; i < SERVO_COUNT_; ++i) {
        _servo[i] = nullptr;
        _last_deg[i] = 90.0f;
    }
}

void ServoManager::_init() {
    ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2); ESP32PWM::allocateTimer(3);
    for (int i = 0; i < SERVO_COUNT_; ++i) {
        _servo[i] = new Servo();
        _servo[i]->setPeriodHertz(50);
        _servo[i]->attach(kPins[i], 500, 2500);
    }
}

void ServoManager::write(ServoId id, float angle_deg) {
    int i = _idx(id);
    if (i < 0 || i >= SERVO_COUNT_ || !_servo[i]) return;
    if (angle_deg < 0) angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;
    _servo[i]->write((int)(angle_deg + 0.5f));
    _last_deg[i] = angle_deg;
}

void ServoManager::write(ServoId id, float angle_deg, float speed_deg_per_s) {
    int i = _idx(id);
    if (i < 0 || i >= SERVO_COUNT_ || !_servo[i]) return;
    if (angle_deg < 0) angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;
    if (speed_deg_per_s <= 0.0f) { write(id, angle_deg); return; }
    float cur = _last_deg[i];
    float dir = (angle_deg > cur) ? 1.0f : -1.0f;
    const float step_deg = 1.0f;
    const uint32_t step_delay_ms = (uint32_t)(1000.0f * step_deg / speed_deg_per_s);
    while (fabsf(angle_deg - cur) > step_deg) {
        cur += dir * step_deg;
        _servo[i]->write((int)(cur + 0.5f));
        _last_deg[i] = cur;
        delay(step_delay_ms);
    }
    _servo[i]->write((int)(angle_deg + 0.5f));
    _last_deg[i] = angle_deg;
}

float ServoManager::read(ServoId id) const {
    int i = _idx(id);
    return (i >= 0 && i < SERVO_COUNT_) ? _last_deg[i] : 0.0f;
}

void ServoManager::detach(ServoId id) {
    int i = _idx(id);
    if (i >= 0 && i < SERVO_COUNT_ && _servo[i]) _servo[i]->detach();
}
