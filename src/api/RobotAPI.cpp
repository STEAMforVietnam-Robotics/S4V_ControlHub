#include "RobotAPI.h"
#include "../ch_config.h"
#include "../devices/Motor.h"
#include "../devices/Servo.h"
#include <Arduino.h>

Robot robot;

Robot::Robot()
    : _left(CH_DRIVE_LEFT_MOTOR),
      _right(CH_DRIVE_RIGHT_MOTOR),
      _default_speed((float)CH_DEFAULT_DRIVE_SPEED),
      _obstacle_threshold_cm((float)CH_OBSTACLE_THRESHOLD_CM),
      _warned_no_distance(false),
      _warned_no_contact(false) {}

void Robot::_init() {
    servo.write((ServoId)CH_GRIPPER_SERVO, (float)CH_GRIPPER_OPEN_DEG);
}

// --- config ---
void Robot::set_drivetrain(MotorId left, MotorId right) { _left = left; _right = right; }
void Robot::set_default_speed(float pct) {
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    _default_speed = pct;
}
float Robot::_resolveSpeed(float r) const {
    if (r < 0)   return _default_speed;
    if (r > 100) return 100;
    return r;
}

// --- motion ---
void Robot::go_forward(float s)  { float v=_resolveSpeed(s); motor.drive_raw(_left,+v); motor.drive_raw(_right,+v); }
void Robot::go_backward(float s) { float v=_resolveSpeed(s); motor.drive_raw(_left,-v); motor.drive_raw(_right,-v); }
void Robot::turn_left(float s)   { float v=_resolveSpeed(s); motor.drive_raw(_left,-v); motor.drive_raw(_right,+v); }
void Robot::turn_right(float s)  { float v=_resolveSpeed(s); motor.drive_raw(_left,+v); motor.drive_raw(_right,-v); }
void Robot::stop() { motor.stop(_left, STOP_COAST); motor.stop(_right, STOP_COAST); }

// --- timing ---
void Robot::wait(float seconds) { if (seconds > 0) delay((uint32_t)(seconds * 1000.0f)); }

// --- manipulator ---
void Robot::grasp()   { servo.write((ServoId)CH_GRIPPER_SERVO, (float)CH_GRIPPER_CLOSE_DEG, 180.0f); }
void Robot::release() { servo.write((ServoId)CH_GRIPPER_SERVO, (float)CH_GRIPPER_OPEN_DEG,  180.0f); }

// --- primary sensor registration ---
void Robot::set_primary_distance(DistanceHandle h) { _primary_distance = h; _warned_no_distance = false; }
void Robot::set_primary_contact (ContactHandle h)    { _primary_contact  = h; _warned_no_contact  = false; }

// --- sensing ---
bool Robot::is_obstacle_detected() {
    if (!_primary_distance.valid()) {
        if (!_warned_no_distance) {
            Serial.println("[ControlHub] robot.is_obstacle_detected(): no primary distance sensor set");
            _warned_no_distance = true;
        }
        return false;
    }
    float d = _primary_distance.distance_cm();
    if (d < 0.0f) return false;            // timeout / OOR → no obstacle
    return d < _obstacle_threshold_cm;
}

bool Robot::is_obstacle_touched() {
    if (!_primary_contact.valid()) {
        if (!_warned_no_contact) {
            Serial.println("[ControlHub] robot.is_obstacle_touched(): no primary contact sensor set");
            _warned_no_contact = true;
        }
        return false;
    }
    return _primary_contact.pressed();
}

void Robot::set_obstacle_threshold_cm(float cm) { if (cm > 0) _obstacle_threshold_cm = cm; }
