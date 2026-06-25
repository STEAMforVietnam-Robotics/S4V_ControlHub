// ============================================================================
//  Robot.h — Level-1 (Mission) API
//
//  Drivetrain wires MOTOR_1 = left, MOTOR_2 = right by default
//  (configurable via robot.set_drivetrain()).
//
//  is_obstacle_detected() and is_obstacle_touched() return false until the
//  user calls set_primary_distance() / set_primary_contact() with a handle
//  created via create(...).
//
//  set_primary_distance() takes a generic DistanceHandle so any of
//  UltrasonicHandle / ToFHandle / SharpIRHandle work (they implicit-convert).
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDistanceSensor.h"
#include "../devices/Contact.h"

class Robot {
public:
    Robot();
    void _init();

    // Drivetrain
    void set_drivetrain(MotorId left, MotorId right);
    void set_default_speed(float pct);

    // Motion
    void go_forward(float speed_pct = -1.0f);
    void go_backward(float speed_pct = -1.0f);
    void turn_left(float speed_pct = -1.0f);
    void turn_right(float speed_pct = -1.0f);
    void stop();

    // Timing
    void wait(float seconds);

    // Manipulator
    void grasp();
    void release();

    // Primary sensor hooks — call once in setup() after create(...).
    void set_primary_distance(DistanceHandle h);
    void set_primary_contact(ContactHandle h);

    // Sensing — return false if no primary sensor was registered.
    bool is_obstacle_detected();
    bool is_obstacle_touched();

    void set_obstacle_threshold_cm(float cm);

private:
    MotorId _left, _right;
    float   _default_speed;
    float   _obstacle_threshold_cm;

    DistanceHandle _primary_distance;
    ContactHandle  _primary_contact;
    bool           _warned_no_distance;
    bool           _warned_no_contact;

    float _resolveSpeed(float requested) const;
};

extern Robot robot;
