/*
 *  L2_MotorControl
 *  ---------------
 *  Level-2 (Action) API: talk to individual motors and sensors directly,
 *  without using the Robot abstraction.
 */

#include <ControlHub.h>

Motor      m0(M1), m1(M2);
Ultrasonic dist(P1);           // HC-SR04 on MF4P_1 (trig/echo)

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    m0.begin();  m1.begin();
    dist.begin();
}

void loop() {
    ControlHub.update();

    uint16_t d = dist.get();
    Serial.printf("Distance = %u mm\n", d);

    if (d > 0 && d < 300) {
        m0.stop();
        m1.stop();
    } else {
        m0.rotate( 60);         // percent, -100..100
        m1.rotate(-60);
    }
    delay(50);
}
