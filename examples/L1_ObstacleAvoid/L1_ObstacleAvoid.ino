/*
 *  L1_ObstacleAvoid
 *  ----------------
 *  A drive-around-obstacle behaviour using only Level-1 Robot API.
 *
 *  Wiring:
 *      Left  motor  -> M1
 *      Right motor  -> M2
 *      HC-SR04      -> MF4P_1 (trig = SIG1, echo = SIG2)
 *      Bumper       -> MF3P_2
 *      Gripper      -> SV1
 */

#include <ControlHub.h>

Motor      leftM (M1);
Motor      rightM(M2);
Servo      gripper(S1);                 // SV1
Ultrasonic dSensor(P1);        // trig = SIG1, echo = SIG2
Contact    bumper (P4);

void setup() {
    ControlHub.begin();

    leftM.begin(); rightM.begin();
    gripper.begin();
    dSensor.begin();
    bumper.begin();

    Robot.setDriveTrain(&leftM, &rightM);
    Robot.invertRight(true);
    Robot.setGripper (&gripper, /*open=*/20, /*closed=*/160);
    Robot.setDistance(&dSensor, /*threshold mm=*/250);
    Robot.setContact (&bumper);

    Robot.release();                  // start with gripper open
}

void loop() {
    ControlHub.update();

    if (Robot.is_obstacle_touched()) {
        // Physical contact - back off and turn
        Robot.go_backward(60); Robot.wait(0.5);
        Robot.turn_left  (60); Robot.wait(0.6);
        return;
    }

    if (Robot.is_obstacle_detected()) {
        // Close object - stop, grasp, back off
        Robot.stop();          Robot.wait(0.2);
        Robot.grasp();         Robot.wait(0.4);
        Robot.go_backward(50); Robot.wait(0.8);
        Robot.release();       Robot.wait(0.3);
        Robot.turn_right(50);  Robot.wait(0.5);
        return;
    }

    Robot.go_forward(70);
}
