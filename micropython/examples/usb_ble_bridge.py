"""usb_ble_bridge.py -- Receive frames from USB and BLE concurrently.

Brings up every transport and installs a global frame handler that prints each
incoming frame's source. On-board modules (if you construct any) still respond
to their own frames automatically; anything unhandled reaches this callback.

Test with the PWA in `webapp/` over BLE, or send raw frames over USB serial.

NOTE: enabling the USB transport disables the REPL's Ctrl-C. Press the board's
reset/EN button to get the REPL back.
"""

from controlhub import control_hub as hub, Motor, Led, M1, M2, P3


def on_frame(frame, source):
    print("frame from %s: %s" % (source, frame))


# Construct the modules you want the host to drive over USB/BLE.
Motor(M1)
Motor(M2)
Led(P3, count=8)

hub.begin(usb=True, ble=True, rs485=True, ble_name="ControlHub")
hub.on_frame(on_frame)
hub.status_ok()

while True:
    hub.update()
