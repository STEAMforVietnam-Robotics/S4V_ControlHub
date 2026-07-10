/*
 *  SmartModule_RS485
 *  -----------------
 *  Control Hub acts as an RS485 master and talks to Smart Modules (ToF,
 *  IMU, Motor Driver, ...) using the frame format from
 *  RVRS485060726162210.pdf.
 *
 *  This example:
 *      1. Every 200 ms sends a status request (RTR=1) to the ToF module
 *         with ID = 0x00 and prints the returned distance.
 *      2. Every 500 ms sends a duty command to Smart Motor Module ID 0x00.
 */

#include <ControlHub.h>

using CH = ControlHubProtocol;

void onRs485Frame(const Frame &f, const char *src) {
    if (strcmp(src, "RS485") != 0) return;

    if (f.moduleType == MOD_TOF && f.msgId == CH::MSG_STATUS) {
        uint16_t mm = ((uint16_t)f.data[0] << 8) | f.data[1];
        Serial.printf("Smart ToF: %u mm  fault=0x%02X\n", mm, f.data[7]);
    }
    else if (f.moduleType == MOD_POWER && f.msgId == CH::MSG_STATUS) {
        float v = f.data[0] * 0.1f;
        int16_t mA = (int16_t)((f.data[1] << 8) | f.data[2]);
        Serial.printf("Power: %.1f V  %d mA\n", v, mA);
    }
    else if (f.moduleType == MOD_MOTOR && f.msgId == CH::MOTOR_MSG_STATUS) {
        int16_t duty = (int16_t)((f.data[0] << 8) | f.data[1]);
        Serial.printf("Smart Motor #%u: duty=%d mode=%u fault=0x%02X\n",
                      f.moduleId, duty, f.data[2], f.data[7]);
    }
}

void setup() {
    Serial.begin(115200);
    // Only need RS485 for this example
    ControlHub.begin(/*USB=*/true, /*BLE=*/false, /*RS485=*/true);
    ControlHub.onFrame(onRs485Frame);
}

void loop() {
    ControlHub.update();

    static uint32_t nextTof = 0;
    if (millis() > nextTof) {
        nextTof = millis() + 200;
        ControlHub.sendRs485(Frame::makeStatusRequest(MOD_TOF, 0x00));
    }

    static uint32_t nextMotor = 0;
    if (millis() > nextMotor) {
        nextMotor = millis() + 500;
        // Command Smart Motor #0 to 60 % clockwise, MODE_DUTY
        Frame cmd = Frame::makeMotorControl(0x00, 6000, MODE_DUTY);
        ControlHub.sendRs485(cmd);
    }
}
