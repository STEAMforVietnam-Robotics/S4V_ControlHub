"""config.py -- Pin mapping & compile-time constants for the Control Hub board
(ESP32-S3 + DRV8874 x4 driven by PCA9685 + 4 servos + multi-function ports).

Ported from src/ControlHubConfig.h and src/ControlHubTypes.h. Change values
here if the schematic is revised so the rest of the library keeps working.
"""

# ---------------------------------------------------------------------------
#  I2C bus (PCA9685 lives here)
# ---------------------------------------------------------------------------
CH_I2C_SDA = 47
CH_I2C_SCL = 21
CH_I2C_FREQ = 400000
CH_PCA9685_ADDR = 0x40
CH_PCA9685_FREQ = 1500          # Hz - DRV8874 accepts up to ~150 kHz

# ---------------------------------------------------------------------------
#  DC Motors -- 4 x DRV8874 driven from PCA9685
#  Each motor uses two PCA9685 channels {IN1 = forward, IN2 = reverse}.
# ---------------------------------------------------------------------------
CH_NUM_MOTORS = 4
# {IN1 channel, IN2 channel} per motor port (1-based key)
CH_MOTOR_CHANNELS = {
    1: (0, 1),
    2: (2, 3),
    3: (15, 14),
    4: (13, 12),
}

# ---------------------------------------------------------------------------
#  Servos - direct ESP32 PWM
# ---------------------------------------------------------------------------
CH_NUM_SERVOS = 4
CH_SERVO_PINS = {1: 11, 2: 12, 3: 38, 4: 39}
CH_SERVO_FREQ = 50              # Hz
CH_SERVO_MIN_US = 500
CH_SERVO_MAX_US = 2500

# ---------------------------------------------------------------------------
#  Multi-function ports
#  MF4P : two signal lines each (SIG1 + SIG2)
#  MF3P : one signal line each  (SIG)
# ---------------------------------------------------------------------------
CH_MF4P_1_SIG1 = 16
CH_MF4P_1_SIG2 = 17
CH_MF4P_2_SIG1 = 18
CH_MF4P_2_SIG2 = 8
CH_MF3P_1_SIG = 3
CH_MF3P_2_SIG = 9
CH_MF3P_3_SIG = 10

# ---------------------------------------------------------------------------
#  WS2812 / NeoPixel on-board status LED
# ---------------------------------------------------------------------------
CH_LED_STATUS_PIN = 40
CH_LED_STATUS_NUM = 12

# ---------------------------------------------------------------------------
#  Gamepad / Phone Sensor (BLE) - disconnect timeout window (ms)
# ---------------------------------------------------------------------------
CH_GAMEPAD_TIMEOUT_MS = 600
CH_PHONE_SENSOR_TIMEOUT_MS = 600

# ---------------------------------------------------------------------------
#  RS485_1  (Smart Module bus - TX=GPIO43 / RX=GPIO44 with DE/RE)
#
#  The RS485 transceiver is wired to GPIO43/44 (the UART0 default pins on the
#  Arduino board). Here we drive those pins with the UART1 *peripheral* routed
#  through the GPIO matrix, NOT UART0: UART0 is held by the system console/REPL,
#  so re-opening UART(0) fails with ESP_ERR_INVALID_STATE. Using UART1 leaves
#  UART0 + the REPL untouched.
# ---------------------------------------------------------------------------
CH_RS485_UART = 0
CH_RS485_TX = 43
CH_RS485_RX = 44
CH_RS485_DE_RE = 2
CH_RS485_BAUD = 115200

# ---------------------------------------------------------------------------
#  RS485_2  (Smart Module bus - UART2 routed to TX=GPIO4 / RX=GPIO5 with DE/RE)
# ---------------------------------------------------------------------------
CH_RS485_2_UART = 2
CH_RS485_2_TX = 4
CH_RS485_2_RX = 5
CH_RS485_2_DE_RE = 6
CH_RS485_2_BAUD = 115200

# ---------------------------------------------------------------------------
#  USB Serial (host <-> hub) -- native USB-CDC, the ONLY wired link to the host
#  (shared with the REPL). There is no separate host UART.
# ---------------------------------------------------------------------------
CH_USB_BAUD = 115200

# ---------------------------------------------------------------------------
#  BLE (Nordic UART Service)
# ---------------------------------------------------------------------------
CH_BLE_DEFAULT_NAME = "ControlHub"
CH_BLE_NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
CH_BLE_NUS_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # host -> hub
CH_BLE_NUS_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # hub  -> host

# ---------------------------------------------------------------------------
#  Frame protocol
# ---------------------------------------------------------------------------
CH_FRAME_HEADER = 0x55
CH_FRAME_MAX_DATA = 8
CH_FRAME_TOTAL_MAX = 1 + 5 + CH_FRAME_MAX_DATA + 1   # 15 bytes

# ===========================================================================
#  Public enums (ported from ControlHubTypes.h) - plain integer constants.
# ===========================================================================

# ----- Motor identifier (1..4) -----
M1, M2, M3, M4 = 1, 2, 3, 4

# ----- Servo identifier (1..4) -----
S1, S2, S3, S4 = 1, 2, 3, 4

# ----- RS485 bus selector -----
RS485_1 = 0
RS485_2 = 1

# ----- Multi-function port identifier -----
PORT_MF4P_1 = 0
PORT_MF4P_2 = 1
PORT_MF3P_1 = 2
PORT_MF3P_2 = 3
PORT_MF3P_3 = 4

# Map port id -> (sig1_pin, sig2_pin_or_None)
CH_PORT_PINS = {
    PORT_MF4P_1: (CH_MF4P_1_SIG1, CH_MF4P_1_SIG2),
    PORT_MF4P_2: (CH_MF4P_2_SIG1, CH_MF4P_2_SIG2),
    PORT_MF3P_1: (CH_MF3P_1_SIG, None),
    PORT_MF3P_2: (CH_MF3P_2_SIG, None),
    PORT_MF3P_3: (CH_MF3P_3_SIG, None),
}

# ----- Motor stopping behaviour -----
STOP_COAST = 0
STOP_BRAKE = 1

# ----- Sensor status -----
SENSOR_OK = 0
SENSOR_NOT_INITIALIZED = 1
SENSOR_OUT_OF_RANGE = 2
SENSOR_TIMEOUT = 3

# ----- Module Type (wire protocol) -----
MOD_SYSTEM = 0x00
MOD_POWER = 0x01
MOD_IMU = 0x02
MOD_TOF = 0x03
MOD_BUMPER = 0x04
MOD_MOTOR = 0x05
MOD_SERVO = 0x06
MOD_ULTRASONIC = 0x07
MOD_SMART_MOTOR = 0x08
MOD_LED = 0x09
MOD_GAMEPAD = 0x0A
MOD_PHONE_SENSOR = 0x0B

# ----- Msg IDs -----
# CONTROL (0x01) / CONFIG (0x02) / STATUS (0x11) are shared by every module
# (Motor, Servo, LED, Gamepad, sensors), so they carry generic names.
MSG_CONTROL = 0x01
MSG_CONFIG = 0x02
MSG_STATUS = 0x11

# Phone Sensor msg ids
PHONE_MSG_ACCEL = 0x01
PHONE_MSG_GYRO = 0x02
PHONE_MSG_MAG = 0x03
PHONE_MSG_GPS = 0x04
PHONE_MSG_GPS_ALT = 0x05

# ----- Motor control modes -----
MODE_DUTY = 0x00
MODE_SPEED = 0x01
MODE_POSITION = 0x02

# ----- Gamepad button bit positions (bits 0..13) -----
GP_A = 0
GP_B = 1
GP_X = 2
GP_Y = 3
GP_UP = 4
GP_DOWN = 5
GP_LEFT = 6
GP_RIGHT = 7
GP_L1 = 8
GP_R1 = 9
GP_R2 = 10
GP_L2 = 11
GP_SELECT = 12
GP_START = 13
