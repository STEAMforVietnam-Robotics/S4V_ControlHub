#include <ControlHub.h>

Motor leftMotor (M1, false);    // M1
Motor rightMotor(M4, true);    // M4
Motor armMotor  (M2);    // M2
Servo gripper (S1);    // S1

Gamepad gp;

void setup() {
    Serial.begin(115200);

    ControlHub.begin(false, true, false, CH_BLE_DEFAULT_NAME);               // start USB + BLE + RS485
    leftMotor.begin();
    rightMotor.begin();
    armMotor.begin();
    gripper.begin();
    gp.begin();

    Robot.setDriveTrain(&leftMotor, &rightMotor);

    gripper.setLimits(70, 120);        // servo angle limits
}

void loop() {
    ControlHub.update(); 
    if(gp.connected()) {
        if(gp.pressed(GP_UP)) {
            Robot.go_forward(50);
        } else if(gp.pressed(GP_DOWN)) {
            Robot.go_backward(50);
        } else if(gp.pressed(GP_LEFT)) {
            Robot.turn_left(50);
        } else if(gp.pressed(GP_RIGHT)) {
            Robot.turn_right(50);
        } else {
            Robot.stop();
        }
        if(gp.justPressed(GP_R1)) {
            gripper.write(70); // close gripper
        } else if(gp.justPressed(GP_R2)) {
            gripper.write(120); // open gripper
        }
        if(gp.pressed(GP_L1)) {
            armMotor.rotate(50); // raise arm
        } else if(gp.pressed(GP_L2)) {
            armMotor.rotate(-50); // lower arm
        } else {
            armMotor.stop(STOP_BRAKE);
        }
    }
    delay(20);
}