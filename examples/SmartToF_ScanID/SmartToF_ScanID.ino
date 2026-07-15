/*
 *  SmartToF_ScanID
 *  ---------------
 *  Quét toàn bộ dải ID để tìm các module Smart ToF đang có mặt trên bus
 *  RS485. Với mỗi ID, Control Hub gửi một yêu cầu status và chờ phản hồi:
 *  nếu module trả lời trong thời gian timeout thì coi như "tìm thấy".
 *
 *  Cách dùng:
 *      - Mở Serial Monitor ở 115200 baud.
 *      - Chương trình sẽ quét ID từ 0x00 đến SCAN_MAX_ID mỗi 3 giây và in
 *        ra danh sách ID phản hồi kèm khoảng cách đọc được.
 */

#include <ControlHub.h>

// Quét ID trong khoảng [0 .. SCAN_MAX_ID]. ID là 8-bit nên tối đa 0xFF,
// nhưng thường chỉ vài module nên đặt nhỏ lại cho nhanh.
static const uint8_t  SCAN_MAX_ID   = 16;
// Thời gian chờ phản hồi cho mỗi ID (ms). Ngắn để quét nhanh, đủ dài để
// module kịp trả lời qua RS485.
static const uint32_t SCAN_TIMEOUT  = 30;
// Bus RS485 để quét: RS485_1 hoặc RS485_2 (mạch có 2 bus smart module).
static const Rs485Bus SCAN_BUS      = RS485_1;

void scanToF() {
    Serial.println(F("---- Quet ID module ToF ----"));
    uint8_t found = 0;

    for (uint16_t id = 0; id <= SCAN_MAX_ID; id++) {
        // Tạo một đối tượng tạm cho ID này và thử đọc.
        SmartToF probe((uint8_t)id, SCAN_BUS);
        uint16_t mm = probe.get(SCAN_TIMEOUT);   // blocking round-trip

        if (probe.status() == 0) {
            // status() == 0  => module có trả lời (không timeout / fault)
            Serial.printf("  [FOUND] ID=0x%02X  distance=%u mm\n",
                          (uint8_t)id, mm);
            found++;
        }
        // Nếu timeout thì bỏ qua, chuyển sang ID kế tiếp.
    }

    if (found == 0) {
        Serial.println(F("  Khong tim thay module ToF nao."));
    } else {
        Serial.printf("  Tong cong: %u module.\n", found);
    }
}

void setup() {
    Serial.begin(115200);
    // Chỉ cần bật RS485 cho việc quét module.
    ControlHub.begin(/*USB=*/true, /*BLE=*/false, /*RS485=*/true);
}

void loop() {
    ControlHub.update();

    static uint32_t nextScan = 0;
    if (millis() > nextScan) {
        nextScan = millis() + 3000;   // quét lại mỗi 3 giây
        scanToF();
    }
}
