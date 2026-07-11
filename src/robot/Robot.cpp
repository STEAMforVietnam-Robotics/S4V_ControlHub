#include "Robot.h"
#include "../modules/Motor.h"
#include "../modules/Servo.h"
#include "../modules/Ultrasonic.h"
#include "../modules/Contact.h"

// Global singleton (users write "Robot.go_forward(50);")
_Robot Robot;

_Robot::_Robot() = default;

void _Robot::begin() { /* nothing yet - motors begin() themselves */ }

void _Robot::setDriveTrain(Motor *left, Motor *right)
{
    _leftM  = left;
    _rightM = right;
}

void _Robot::setGripper(Servo *s, uint8_t openA, uint8_t closedA)
{
    _gripper   = s;
    _openAng   = openA;
    _closedAng = closedA;
}

void _Robot::setDistance(Ultrasonic *s, uint16_t threshold)
{
    _dist          = s;
    _distThreshold = threshold;
}

void _Robot::setContact(Contact *s) { _contact = s; }

// ---------------------------------------------------------------------------
//  Movement primitives
// ---------------------------------------------------------------------------
void _Robot::_driveRaw(int16_t left, int16_t right)
{
    if (!_leftM || !_rightM) return;    // setDriveTrain() never called
    _leftM ->rotate(left);
    _rightM->rotate(right);
}

void _Robot::wait(float seconds)
{
    delay((uint32_t)(seconds * 1000.0f));
}

void _Robot::stop()
{
    _driveRaw(0, 0);
    // Don't touch the gripper - "stop the robot" only affects motion.
}

void _Robot::go_forward (int16_t s) { _driveRaw( s,  s); }
void _Robot::go_backward(int16_t s) { _driveRaw(-s, -s); }
void _Robot::turn_left  (int16_t s) { _driveRaw(-s,  s); }
void _Robot::turn_right (int16_t s) { _driveRaw( s, -s); }

// ---------------------------------------------------------------------------
//  Gripper
// ---------------------------------------------------------------------------
void _Robot::grasp()
{
    if (_gripper) _gripper->write(_closedAng);
}
void _Robot::release()
{
    if (_gripper) _gripper->write(_openAng);
}

// ---------------------------------------------------------------------------
//  Sensors
// ---------------------------------------------------------------------------
bool _Robot::is_obstacle_detected()
{
    if (!_dist) return false;
    uint16_t d = _dist->get();
    return (d > 0) && (d < _distThreshold);
}

bool _Robot::is_obstacle_touched()
{
    if (!_contact) return false;
    return _contact->get();
}
