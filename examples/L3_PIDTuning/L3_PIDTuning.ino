/*
 * L3_PIDTuning — Level 3 (HARD) example
 *
 * Closed-loop speed control on M1, pot-tunable PID gains, status LED
 * lighting up when the loop error exceeds a threshold.
 */
#include <ControlHub.h>

PotHandle        kp_pot;
DigitalOutHandle status;

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    kp_pot = create(POT,         PORT_MF3P_1);
    status = create(DIGITAL_OUT, PORT_MF3P_2, /*initial=*/false);

    motor.set_I(MOTOR_1, 0.10f);
    motor.set_D(MOTOR_1, 0.00f);
    motor.set_controlmode(MOTOR_1, MODE_SPEED);
    Sys.wait(0.5);
}

void loop() {
    // Live-tune Kp from the potentiometer (0..2.0).
    float kp = kp_pot.scaled(0.0f, 2.0f);
    motor.set_P(MOTOR_1, kp);

    // Step setpoint every 3 s.
    static const float steps[] = { 0, 50, 80, 30, -50, 0 };
    static uint8_t idx = 0;
    static uint32_t t0 = millis();
    if (millis() - t0 > 3000) {
        idx = (idx + 1) % (sizeof(steps) / sizeof(steps[0]));
        motor.set_speed(MOTOR_1, steps[idx]);
        t0 = millis();
    }

    float meas = motor.get_speed(MOTOR_1);
    float err  = fabsf(steps[idx] - meas);
    status.write(err > 10.0f);                  // LED on when out-of-spec

    Serial.print("kp:");   Serial.print(kp, 2);
    Serial.print("\tsp:"); Serial.print(steps[idx], 1);
    Serial.print("\tmeas:"); Serial.println(meas, 1);
    Sys.wait_millisec(50);
}
