# ControlHub BLE Tester (PWA)

Web Bluetooth PWA để test giao tiếp BLE với **ControlHub** (ESP32-S3, Nordic UART Service).
Điều khiển trực quan Motor / Servo / LED / Gamepad và xem log frame gửi/nhận.

## Yêu cầu
- **Chrome hoặc Edge** (desktop, hoặc Android). Safari/Firefox **không** hỗ trợ Web Bluetooth.
- **Secure context**: `localhost` hoặc HTTPS. Mở trực tiếp `file://` sẽ không đăng ký được
  service worker và một số trình duyệt chặn Bluetooth — hãy chạy qua static server.

## Chạy local
Từ trong thư mục `webapp/`:

```bash
# Python
python -m http.server 8000
# hoặc Node
npx serve -l 8000
```

Rồi mở **http://localhost:8000** bằng Chrome/Edge → bấm **Kết nối** → chọn thiết bị `ControlHub`.

> Trên ControlHub, nạp một sketch có bật BLE, ví dụ `examples/USB_BLE_Bridge` (advertise tên
> `ControlHub` hoặc `ControlHub-Demo`).

## Cài như app (Install)
Khi mở qua HTTPS (hoặc localhost), trình duyệt hiện nút **Cài đặt** — app chạy standalone,
mở lại được offline (app shell được cache; chức năng BLE vẫn cần thiết bị thật).

## Giao thức
Frame: `0x55 | modType | modId | msgId | rtr | dlc | data[0..8] | crc8` (CRC-8/SMBUS, gồm cả
header). Chi tiết trong `js/protocol.js`. UUID NUS:
- Service `6e400001-…`, RX (write) `6e400002-…`, TX (notify) `6e400003-…`.

## Kiểm tra nhanh protocol
Mở DevTools Console — khi load, một self-test in ra `[selftest] ...`. Bạn cũng có thể gõ:

```js
CHProto.toHex(CHProto.frameMotor(0, 50, 0)); // "55 05 00 01 00 03 13 88 00 xx"
```

khớp với ví dụ trong `examples/USB_BLE_Bridge`.

## Cấu trúc
```
webapp/
├─ index.html            # markup + panel
├─ css/styles.css        # theme tối, responsive
├─ js/protocol.js        # crc8, buildFrame, FrameParser, hằng số
├─ js/ble.js             # lớp Web Bluetooth
├─ js/app.js             # UI ↔ BLE ↔ protocol, log, gamepad streaming, PWA
├─ manifest.webmanifest
├─ sw.js                 # service worker cache-first
└─ icons/                # icon 192/512
```
