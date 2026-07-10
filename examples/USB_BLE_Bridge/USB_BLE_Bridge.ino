/*
 *  USB_BLE_Bridge
 *  --------------
 *  Demonstrates simultaneous host control over USB Serial and BLE. Either
 *  side (or both at once) can send binary frames to drive the on-board
 *  motors using the exact RS485 protocol format documented in
 *  RVRS485060726162210.pdf.
 *
 *  Frame layout (from PDF):
 *      0x55 | ModuleType | ModuleID | MsgID | RTR | DLC | Data... | CRC8
 *
 *  Example: run motor #0 forward at 50 %
 *      55 05 00 01 00 03 13 88 00 <CRC>
 *      ^header                              (0x1388 = 5000 = 50.00 %)
 *
 *  Try it:
 *      * From a PC:  python send_frame.py    (any script that writes those
 *                                             bytes to the USB CDC port)
 *      * From phone: nRF Connect -> connect to "ControlHub" -> write to the
 *                    Nordic UART RX characteristic.
 */

#include <ControlHub.h>

Motor m0(M1);
Motor m1(M2);

void onHostFrame(const Frame &f, const char *src) {
    Serial.printf("[%s] frame  type=0x%02X id=%u msg=0x%02X rtr=%u dlc=%u\n",
                  src, f.moduleType, f.moduleId, f.msgId, f.rtr, f.dlc);
}

void setup() {
    Serial.begin(115200);

    // Turn on all three transports and start listening.
    ControlHub.begin(/*USB=*/true, /*BLE=*/true, /*RS485=*/false, "ControlHub-Demo");

    m0.begin(); m1.begin();

    // Optional: peek at every host frame that arrives (useful for teaching)
    ControlHub.onFrame(onHostFrame);
}

void loop() {
    ControlHub.update();

    // Every 500 ms broadcast motor #0's status so both hosts can see it.
    static uint32_t next = 0;
    if (millis() > next) {
        next = millis() + 500;
        ControlHub.sendFrame(m0.makeStatusFrame());
    }
}
