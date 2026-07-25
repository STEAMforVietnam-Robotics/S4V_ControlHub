/*
 *  PhoneSensor_Serial
 *  -------------------
 *  Nhận frame cảm biến điện thoại (Accel/Gyro/Mag/GPS) qua BLE (Nordic UART
 *  Service) và in ra Serial Monitor. Dùng để kiểm tra app điện thoại đang gửi
 *  đúng frame MOD_PHONE_SENSOR vào Control Hub.
 *
 *  Cách dùng:
 *      1. Nạp sketch, mở Serial Monitor @115200.
 *      2. Kết nối app điện thoại tới "ControlHub" qua BLE.
 *      3. Bật gửi Accel/Gyro/Mag/GPS trên app -> dữ liệu in ra Serial.
 *
 *  Wire format (MOD_PHONE_SENSOR, xem PhoneSensor.h):
 *      PHONE_MSG_ACCEL   accelX/Y/Z (g)
 *      PHONE_MSG_GYRO    gyroX/Y/Z  (deg/s)
 *      PHONE_MSG_MAG     magX/Y/Z   (uT)
 *      PHONE_MSG_GPS     lat/lon    (deg)
 *      PHONE_MSG_GPS_ALT altitude   (m)
 */

#include <ControlHub.h>

PhoneSensor phone(1);   // wire id = 1 (điện thoại đầu tiên)

// Gọi mỗi khi có frame cảm biến mới đến (bất kỳ loại nào).
void onPhoneUpdate(PhoneSensor &p) {
    Serial.print("accel(g)=(");
    Serial.print(p.accelX(), 3); Serial.print(',');
    Serial.print(p.accelY(), 3); Serial.print(',');
    Serial.print(p.accelZ(), 3); Serial.print(")");

    Serial.print("  gyro(dps)=(");
    Serial.print(p.gyroX(), 1); Serial.print(',');
    Serial.print(p.gyroY(), 1); Serial.print(',');
    Serial.print(p.gyroZ(), 1); Serial.print(")");

    Serial.print("  mag(uT)=(");
    Serial.print(p.magX(), 1); Serial.print(',');
    Serial.print(p.magY(), 1); Serial.print(',');
    Serial.print(p.magZ(), 1); Serial.print(")");

    Serial.print("  gps=(");
    Serial.print(p.latitude(), 6); Serial.print(',');
    Serial.print(p.longitude(), 6); Serial.print(')');
    Serial.print(" alt="); Serial.print(p.altitude(), 1); Serial.println('m');
}

void setup() {
    Serial.begin(115200);
    ControlHub.begin();     // bật USB + BLE + RS485; BLE quảng bá "ControlHub"
    phone.begin();
    phone.onUpdate(onPhoneUpdate);
    Serial.println("PhoneSensor_Serial: cho ket noi va gui frame cam bien...");
}

void loop() {
    ControlHub.update();    // giữ cho parser BLE chạy -> gọi onPhoneUpdate khi có frame

    // Cảnh báo khi mất kết nối (không nhận frame trong CH_PHONE_SENSOR_TIMEOUT_MS).
    static bool wasConnected = false;
    bool now = phone.connected();
    if (now != wasConnected) {
        Serial.println(now ? "[phone CONNECTED]" : "[phone TIMEOUT]");
        wasConnected = now;
    }

    delay(5);
}
