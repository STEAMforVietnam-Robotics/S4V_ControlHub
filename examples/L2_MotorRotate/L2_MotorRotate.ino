/*
 * L2_MotorRotate — Level 2 (MEDIUM) example
 *
 * Uses action-level motor.*, Sys.* and a directly-handled distance sensor.
 * Two sonars (front + side), a potentiometer to control speed.
 */
#include <ControlHub.h>

UltrasonicHandle front_sonar;
UltrasonicHandle side_sonar;
PotHandle        speed_pot;

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    front_sonar = create(ULTRASONIC, PORT_MF4P_1);
    side_sonar  = create(ULTRASONIC, PORT_MF4P_2);
    speed_pot   = create(POT,        PORT_MF3P_1);

    Sys.wait_sec(0.5);
    motor.reset_encoder(MOTOR_1);
}

void loop() {
    float pct = speed_pot.percent();              // 0..100

    Serial.print("front="); Serial.print(front_sonar.distance_cm(), 1);
    Serial.print("cm  side="); Serial.print(side_sonar.distance_cm(), 1);
    Serial.print("cm  speed="); Serial.print(pct, 1);
    Serial.print("%  M1_pos="); Serial.println(motor.position(MOTOR_1), 1);

    // Rotate M1 by 90 degrees at the pot-controlled speed.
    motor.rotate(MOTOR_1, 90.0f, pct);
    Sys.wait_sec(1.0);
}
