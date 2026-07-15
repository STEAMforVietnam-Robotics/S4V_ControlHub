/*
 *  SmartModule_RS485
 *  -----------------
 *  Control Hub acts as an RS485 master and talks to Smart Modules (ToF,
 *  Motor Driver, ...) using the frame format from RVRS485060726162210.pdf.
 *
 *  This example:
 *      1. Every 200 ms polls the Smart ToF module (ID = 0x00) via
 *         SmartToF::get() and prints the returned distance.
 *      2. Every 500 ms sends a duty command to Smart Motor Module ID 0x00.
 */

#include <ControlHub.h>

// Smart ToF module ID 0x00 trên bus RS485_1.
// Đổi sang RS485_2 nếu module cắm ở bus thứ hai:  SmartToF tof(0x00, RS485_2);
SmartToF tof(0x00, RS485_1);

// Smart Bumper module ID 0x00 trên bus RS485_1.
SmartBumper bumper(0x00, RS485_1);

void setup() {
    Serial.begin(115200);
    // Only need RS485 for this example
    ControlHub.begin(/*USB=*/true, /*BLE=*/false, /*RS485=*/true);
    tof.begin();
    bumper.begin();
}

void loop() {
    ControlHub.update();

    static uint32_t nextTof = 0;
    if (millis() > nextTof) {
        nextTof = millis() + 200;
        uint16_t mm = tof.get();   // blocking round-trip over RS485
        if (tof.status() == 0) {
            Serial.printf("Smart ToF: %u mm\n", mm);
        } else {
            Serial.println("Smart ToF: timeout");
        }
    }

    static uint32_t nextBumper = 0;
    if (millis() > nextBumper) {
        nextBumper = millis() + 200;
        bool pressed = bumper.get();   // blocking round-trip over RS485
        if (bumper.status() == 0) {
            Serial.printf("Smart Bumper: %s\n", pressed ? "PRESSED" : "released");
        } else {
            Serial.println("Smart Bumper: timeout");
        }
    }

    static uint32_t nextMotor = 0;
    if (millis() > nextMotor) {
        nextMotor = millis() + 500;
        // Command Smart Motor #0 to 60 % clockwise, MODE_DUTY
        Frame cmd = Frame::makeMotorControl(0x00, 6000, MODE_DUTY);
        ControlHub.sendRs485(cmd);
    }
}
