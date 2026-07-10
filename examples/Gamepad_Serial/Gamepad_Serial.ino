/*
 *  Gamepad_Serial
 *  --------------
 *  Nhận frame gamepad qua BLE (Nordic UART Service) và in trạng thái nút +
 *  joystick ra Serial Monitor. Dùng để kiểm tra web BLE Tester (webapp/) hoặc
 *  bất kỳ app nào gửi MOD_GAMEPAD frame vào Control Hub.
 *
 *  Cách dùng:
 *      1. Nạp sketch, mở Serial Monitor @115200.
 *      2. Kết nối tới "ControlHub" bằng webapp/ (panel Gamepad) hoặc nRF Connect.
 *      3. Bấm nút / kéo joystick -> trạng thái in ra Serial.
 *
 *  Wire format (MOD_GAMEPAD, MOTOR_MSG_CONTROL, dlc = 6):
 *      data[0] = buttons hi,  data[1] = buttons lo
 *      data[2] = leftX,  data[3] = leftY,  data[4] = rightX,  data[5] = rightY
 *      (trục 0..255, 128 = giữa; API quy về -100..100 với deadzone ~8%)
 */

#include <ControlHub.h>

Gamepad gp(1);          // wire id = 1 (gamepad đầu tiên)

// In tên các nút đang được giữ.
void printButtons() {
    struct { const char *name; uint8_t bit; } btns[] = {
        {"A", GP_A}, {"B", GP_B}, {"X", GP_X}, {"Y", GP_Y},
        {"UP", GP_UP}, {"DOWN", GP_DOWN}, {"LEFT", GP_LEFT}, {"RIGHT", GP_RIGHT},
        {"L1", GP_L1}, {"R1", GP_R1}, {"L2", GP_L2}, {"R2", GP_R2},
        {"SELECT", GP_SELECT}, {"START", GP_START},
    };

    Serial.print("btn=[");
    bool any = false;
    for (auto &b : btns) {
        if (gp.pressed(b.bit)) {
            if (any) Serial.print(' ');
            Serial.print(b.name);
            any = true;
        }
    }
    if (!any) Serial.print("-");
    Serial.print(']');
}

// Gọi mỗi khi có frame gamepad mới đến.
void onGamepad(Gamepad &g) {
    printButtons();
    Serial.print("  L(");
    Serial.print(g.leftX());  Serial.print(',');  Serial.print(g.leftY());
    Serial.print(")  R(");
    Serial.print(g.rightX()); Serial.print(','); Serial.print(g.rightY());
    Serial.print(")  mask=0x");
    Serial.println(g.buttons(), HEX);

    // Ví dụ bắt sự kiện nhấn (rising edge):
    if (g.justPressed(GP_A))     Serial.println(">> A pressed");
    if (g.justPressed(GP_START)) Serial.println(">> START pressed");
}

void setup() {
    Serial.begin(115200);
    ControlHub.begin();     // bật USB + BLE + RS485; BLE quảng bá "ControlHub"
    gp.begin();
    gp.onUpdate(onGamepad);
    Serial.println("Gamepad_Serial: cho ket noi va gui frame gamepad...");
}

void loop() {
    ControlHub.update();    // giữ cho parser BLE chạy -> gọi onGamepad khi có frame

    // Cảnh báo khi mất kết nối gamepad (không nhận frame trong CH_GAMEPAD_TIMEOUT_MS).
    static bool wasConnected = false;
    bool now = gp.connected();
    if (now != wasConnected) {
        Serial.println(now ? "[gamepad CONNECTED]" : "[gamepad TIMEOUT]");
        wasConnected = now;
    }

    delay(5);
}
