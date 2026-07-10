/*
 *  L1_HelloRobot
 *  -------------
 *  Simplest possible Control Hub sketch. Drives the robot in a square using
 *  the Level-1 Robot API. No sensors, no host control.
 *
 *  Wiring:
 *      Left  motor -> M1 (encoder port)
 *      Right motor -> M2 (encoder port)
 */

#include <ControlHub.h>

Motor leftMotor (M1);    // M1
Motor rightMotor(M2);    // M2

void setup() {
    Serial.begin(115200);

    ControlHub.begin();               // start USB + BLE + RS485
    leftMotor.begin();
    rightMotor.begin();

    Robot.setDriveTrain(&leftMotor, &rightMotor);
    Robot.invertRight(true);          // right motor mirror-mounted
}

void loop() {
    ControlHub.update();              // keep host protocol alive

    // Drive in a square
    Robot.go_forward(60);   Robot.wait(1.0);
    Robot.turn_right(50);   Robot.wait(0.5);
    Robot.stop();           Robot.wait(0.3);
}
