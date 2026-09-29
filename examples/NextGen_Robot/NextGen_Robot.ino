#include <ControlHub.h>


#define LED_NUM 160

Motor leftMotor (M2, true);    // M1
Motor rightMotor(M4, false);    // M4
Motor armMotor  (M1);    // M2
Servo gripper (S1);    // S1
Led   led(P3, LED_NUM);    // P3

Gamepad gp;

void setup() {
    Serial.begin(115200);

    ControlHub.begin(false, true, false, CH_BLE_DEFAULT_NAME);               // start USB + BLE + RS485
    leftMotor.begin();
    rightMotor.begin();
    armMotor.begin();
    gripper.begin();
    led.begin();
    gp.begin();

    Robot.setDriveTrain(&leftMotor, &rightMotor);

    gripper.setLimits(70, 120);        // servo angle limits

    led.setBrightness(255);
    led.setColor(0, 0, 255);    // blue
}

void loop() {
    ControlHub.update(); 
    if(gp.connected()) {
        if(gp.pressed(GP_UP)) {
            Robot.go_forward(70);
        } else if(gp.pressed(GP_DOWN)) {
            Robot.go_backward(60);
        } else if(gp.pressed(GP_LEFT)) {
            Robot.turn_left(65);
        } else if(gp.pressed(GP_RIGHT)) {
            Robot.turn_right(65);
        } else {
            Robot.stop();
        }
        if(gp.justPressed(GP_B)) {
            gripper.write(70); // close gripper
        } else if(gp.justPressed(GP_X)) {
            gripper.write(120); // open gripper
        }
        if(gp.pressed(GP_Y)) {
            armMotor.rotate(80); // raise arm
        } else if(gp.pressed(GP_A)) {
            armMotor.rotate(-50); // lower arm
        } else {
            armMotor.stop(STOP_BRAKE);
        }
    }
    delay(20);
}