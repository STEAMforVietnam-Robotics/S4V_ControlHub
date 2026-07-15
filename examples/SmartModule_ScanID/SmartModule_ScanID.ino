/*
 *  SmartModule_ScanID
 *  ------------------
 *  Quét toàn bộ dải ID cho TẤT CẢ các loại Smart Module trên bus RS485.
 *  Với mỗi (moduleType, id), Control Hub gửi một yêu cầu status và chờ phản
 *  hồi: nếu module trả lời trong thời gian timeout thì coi như "tìm thấy".
 *
 *  Khác với SmartToF_ScanID (chỉ quét ToF), example này dùng trực tiếp
 *  ControlHub.comm().requestRs485() nên quét được mọi loại module mà không
 *  cần một class probe riêng cho từng loại.
 *
 *  Cách dùng:
 *      - Mở Serial Monitor ở 115200 baud.
 *      - Chương trình quét ID từ 0x00 đến SCAN_MAX_ID cho từng loại module,
 *        mỗi 3 giây, và in ra danh sách ID phản hồi.
 */

#include <ControlHub.h>

using namespace ControlHubProtocol;

// Quét ID trong khoảng [0 .. SCAN_MAX_ID]. ID là 8-bit nên tối đa 0xFF,
// nhưng thường chỉ vài module nên đặt nhỏ lại cho nhanh.
static const uint8_t  SCAN_MAX_ID  = 16;
// Thời gian chờ phản hồi cho mỗi ID (ms).
static const uint32_t SCAN_TIMEOUT = 30;
// Bus RS485 để quét: RS485_1 hoặc RS485_2 (mạch có 2 bus smart module).
static const Rs485Bus SCAN_BUS     = RS485_1;

// Danh sách các loại module cần quét trên bus RS485.
struct ModuleKind {
    uint8_t     type;
    const char *name;
};
static const ModuleKind KINDS[] = {
    { MOD_TOF,         "ToF"         },
    { MOD_BUMPER,      "Bumper"      },
    { MOD_MOTOR,       "Motor"       },
    { MOD_SMART_MOTOR, "SmartMotor"  },
    { MOD_ULTRASONIC,  "Ultrasonic"  },
    { MOD_IMU,         "IMU"         },
    { MOD_POWER,       "Power"       },
    { MOD_LED,         "LED"         },
};
static const uint8_t KIND_COUNT = sizeof(KINDS) / sizeof(KINDS[0]);

// Gửi request tới đúng bus đang chọn.
static bool probe(uint8_t type, uint8_t id, Frame &reply) {
    return (SCAN_BUS == RS485_2)
               ? ControlHub.comm().requestRs485_2(type, id, reply, SCAN_TIMEOUT)
               : ControlHub.comm().requestRs485  (type, id, reply, SCAN_TIMEOUT);
}

void scanAll() {
    Serial.println(F("==== Quet ID tat ca module tren bus RS485 ===="));
    uint8_t total = 0;

    for (uint8_t k = 0; k < KIND_COUNT; k++) {
        Serial.printf("-- %s (type=0x%02X) --\n", KINDS[k].name, KINDS[k].type);
        uint8_t found = 0;

        for (uint16_t id = 0; id <= SCAN_MAX_ID; id++) {
            Frame reply;
            if (probe(KINDS[k].type, (uint8_t)id, reply)) {
                Serial.printf("  [FOUND] %s ID=0x%02X\n", KINDS[k].name, (uint8_t)id);
                found++;
            }
        }

        if (found == 0) Serial.println(F("  (khong co)"));
        total += found;
    }

    Serial.printf("Tong cong: %u module.\n\n", total);
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
        scanAll();
    }
}
