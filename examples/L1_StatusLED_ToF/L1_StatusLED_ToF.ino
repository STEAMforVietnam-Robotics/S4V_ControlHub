/*
 * L1_StatusLED_ToF — Phase-3 showcase
 *
 * Uses:
 *   - ToF (VL53L0X) on PORT_MF4P_1 as the primary distance sensor
 *   - Contact bumper on PORT_MF3P_3
 *   - NeoPixel ring (8 px) on PORT_MF3P_1 as status display
 *   - Buzzer on PORT_MF3P_2 for collision alert
 *
 * Behaviour:
 *   - Green ring while driving freely
 *   - Yellow ring + slower drive when an obstacle is < 30 cm
 *   - Red ring + beep + back-up when bumper triggered
 */
#include <ControlHub.h>

ToFHandle      tof;
NeoPixelHandle leds;
BuzzerHandle   buzz;
ContactHandle  bumper;

void set_ring(uint8_t r, uint8_t g, uint8_t b) {
    leds.fill(r, g, b);
    leds.show();
}

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    tof    = create(TOF,      PORT_MF4P_1);
    leds   = create(NEOPIXEL, PORT_MF3P_1, 8);
    buzz   = create(BUZZER,   PORT_MF3P_2);
    bumper = create(CONTACT,  PORT_MF3P_3);

    robot.set_primary_distance(tof);          // ToF → DistanceHandle (implicit)
    robot.set_primary_contact(bumper);
    robot.set_obstacle_threshold_cm(30);
    leds.set_brightness(80);
    set_ring(0, 64, 0);                       // ready = green
}

void loop() {
    if (robot.is_obstacle_touched()) {
        set_ring(64, 0, 0);
        buzz.tone(880, 200);
        robot.stop();
        robot.wait(0.3);
        robot.go_backward(60);
        robot.wait(0.6);
    } else if (robot.is_obstacle_detected()) {
        set_ring(64, 48, 0);                  // amber
        robot.go_forward(25);
    } else {
        set_ring(0, 64, 0);
        robot.go_forward(70);
    }
    robot.wait(0.05);
}
