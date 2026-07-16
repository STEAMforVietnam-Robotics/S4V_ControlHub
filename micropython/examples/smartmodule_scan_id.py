"""smartmodule_scan_id.py -- Quét ID mọi loại Smart Module trên bus RS485.

Với mỗi (module_type, id), Control Hub gửi một yêu cầu status (RTR=1) và chờ
phản hồi; nếu module trả lời trong thời gian timeout thì coi như "tìm thấy".

Dùng trực tiếp hub.comm.request_rs485() nên quét được mọi loại module mà không
cần class probe riêng cho từng loại (port từ SmartModule_ScanID.ino).

Cách dùng:
    - Nạp `controlhub/` lên board, chạy file này qua USB.
    - Chương trình quét ID [0 .. SCAN_MAX_ID] cho từng loại module, mỗi 3 giây,
      và in danh sách ID phản hồi ra USB.
"""

import time

from controlhub import (
    control_hub as hub,
    MOD_TOF, MOD_BUMPER, MOD_MOTOR, MOD_SMART_MOTOR, MOD_ULTRASONIC,
    MOD_IMU, MOD_POWER, MOD_LED,
    RS485_1, RS485_2,
)

# Quét ID trong khoảng [0 .. SCAN_MAX_ID]. ID là 8-bit (tối đa 0xFF) nhưng
# thường chỉ vài module nên đặt nhỏ cho nhanh.
SCAN_MAX_ID = 16
# Thời gian chờ phản hồi cho mỗi ID (ms).
SCAN_TIMEOUT = 30
# Bus RS485 để quét: RS485_1 hoặc RS485_2.
SCAN_BUS = RS485_1

# Các loại module cần quét: (module_type, tên).
KINDS = (
    (MOD_TOF, "ToF"),
    (MOD_BUMPER, "Bumper"),
    (MOD_MOTOR, "Motor"),
    (MOD_SMART_MOTOR, "SmartMotor"),
    (MOD_ULTRASONIC, "Ultrasonic"),
    (MOD_IMU, "IMU"),
    (MOD_POWER, "Power"),
    (MOD_LED, "LED"),
)


def probe(mod_type, mod_id):
    """Gửi request tới đúng bus đang chọn; trả về Frame phản hồi hoặc None."""
    if SCAN_BUS == RS485_2:
        return hub.comm.request_rs485_2(mod_type, mod_id, SCAN_TIMEOUT)
    return hub.comm.request_rs485(mod_type, mod_id, SCAN_TIMEOUT)


def scan_all():
    print("==== Quet ID tat ca module tren bus RS485 ====")
    total = 0
    for mod_type, name in KINDS:
        print("-- %s (type=0x%02X) --" % (name, mod_type))
        found = 0
        for mod_id in range(SCAN_MAX_ID + 1):
            if probe(mod_type, mod_id) is not None:
                print("  [FOUND] %s ID=0x%02X" % (name, mod_id))
                found += 1
        if found == 0:
            print("  (khong co)")
        total += found
    print("Tong cong: %d module.\n" % total)


# Chỉ cần bật RS485 cho việc quét module (tắt BLE cho nhẹ).
hub.begin(usb=True, ble=False, rs485=True)

next_scan = 0
while True:
    hub.update()

    now = time.ticks_ms()
    if time.ticks_diff(now, next_scan) >= 0:
        next_scan = time.ticks_add(now, 3000)   # quét lại mỗi 3 giây
        scan_all()
