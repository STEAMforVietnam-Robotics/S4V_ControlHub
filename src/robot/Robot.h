/*  Robot.h
 *
 *  LEVEL 1 - "Mission" API.
 *  The Robot object glues together a pair of drive motors, a gripper servo,
 *  a distance sensor and a contact sensor into simple verbs that beginners
 *  can use directly (go_forward, turn_left, is_obstacle_detected, ...).
 *
 *  It is intentionally light on options: any user who needs more control
 *  drops down to the Motor / Servo / Ultrasonic / Contact objects directly
 *  (Level 2 / 3 API).
 *
 *  A single global @c Robot instance is defined in Robot.cpp so sketches
 *  can write @c Robot.go_forward(50) with no extra setup.
 */

#ifndef CONTROLHUB_ROBOT_H
#define CONTROLHUB_ROBOT_H

#include <Arduino.h>

class Motor;
class Servo;
class Ultrasonic;
class Contact;

class _Robot {
public:
    _Robot();

    /** Bind the objects that make up the "robot". Call from setup(). */
    void begin();

    // ---- Configuration hooks --------------------------------------
    /** Attach two already-constructed Motor objects (no ownership taken). */
    void setDriveTrain(Motor *left, Motor *right);

    /** Optionally invert one side (mount the motor upside down? no worries). */
    void invertLeft (bool inv) { _leftInv  = inv; }
    void invertRight(bool inv) { _rightInv = inv; }

    /** Attach a gripper servo and its open/closed angles. */
    void setGripper(Servo *servo, uint8_t openAngle = 20, uint8_t closedAngle = 160);

    /** Attach an ultrasonic sensor. Threshold in mm for is_obstacle_detected(). */
    void setDistance(Ultrasonic *sensor, uint16_t thresholdMm = 200);

    /** Attach a contact / bumper sensor. */
    void setContact (Contact  *sensor);

    // ---- Level 1 API ----------------------------------------------
    /** Wait @p seconds. Same effect as system.wait_sec(). */
    void wait(float seconds);

    /** Immediate stop of the drivetrain and the gripper. */
    void stop();

    void go_forward (int16_t speedPercent = 50);
    void go_backward(int16_t speedPercent = 50);
    void turn_left  (int16_t speedPercent = 50);
    void turn_right (int16_t speedPercent = 50);

    void grasp();
    void release();

    bool is_obstacle_detected();
    bool is_obstacle_touched();

private:
    void _driveRaw(int16_t left, int16_t right);

    Motor    *_leftM   = nullptr;
    Motor    *_rightM  = nullptr;
    bool      _leftInv  = false;
    bool      _rightInv = true;    // default: right motor mirror-mounted

    Servo    *_gripper       = nullptr;
    uint8_t   _openAng       = 20;
    uint8_t   _closedAng     = 160;

    Ultrasonic *_dist        = nullptr;
    uint16_t  _distThreshold = 200;

    Contact  *_contact       = nullptr;
};

extern _Robot Robot;

#endif
