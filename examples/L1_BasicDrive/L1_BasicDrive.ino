/*
 * L1_BasicDrive — Level 1 (EASY) example
 *
 * Demonstrates the new create() pattern with the high-level robot.* API.
 * Hardware: HC-SR04 on PORT_MF4P_1, bumper switch on PORT_MF3P_1.
 */
#include <ControlHub.h>

// Declare handles as globals (empty) — assigned in setup() after begin().
UltrasonicHandle sonar;
ContactHandle    bumper;

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    // Create devices now that Serial + hardware are up; any port error prints
    // to Serial immediately.
    sonar  = create(ULTRASONIC, PORT_MF4P_1);
    bumper = create(CONTACT,    PORT_MF3P_1);

    robot.set_primary_distance(sonar);
    robot.set_primary_contact(bumper);
    robot.set_default_speed(60);
    robot.release();
}

void loop() {
    if (robot.is_obstacle_touched()) {
        robot.stop();
        robot.wait(0.3);
        robot.go_backward();
        robot.wait(0.5);
    } else if (robot.is_obstacle_detected()) {
        robot.stop();
        robot.wait(0.2);
        robot.turn_right();
        robot.wait(0.4);
    } else {
        robot.go_forward();
    }
    robot.wait(0.05);
}
