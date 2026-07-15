/*  ControlHubConfig.h
 *  Pin mapping & compile-time configuration for the Control Hub board
 *  (ESP32-S3 + DRV8874 x6 driven by PCA9685 + 4 servos + multi-function ports).
 *
 *  All values come from ControlHub.md - change them here if the schematic
 *  is revised so the rest of the library keeps compiling unchanged.
 */

#ifndef CONTROLHUB_CONFIG_H
#define CONTROLHUB_CONFIG_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
//  I2C bus (PCA9685 lives here)
// ---------------------------------------------------------------------------
#ifndef CH_I2C_SDA
#define CH_I2C_SDA          47
#endif
#ifndef CH_I2C_SCL
#define CH_I2C_SCL          21
#endif
#ifndef CH_I2C_FREQ
#define CH_I2C_FREQ         400000UL
#endif
#ifndef CH_PCA9685_ADDR
#define CH_PCA9685_ADDR     0x40
#endif
#ifndef CH_PCA9685_FREQ
#define CH_PCA9685_FREQ     1500    // Hz - DRV8874 accepts up to ~150kHz
#endif

// ---------------------------------------------------------------------------
//  DC Motors  --  6 x DRV8874 driven from PCA9685
//  Each motor uses two PCA9685 channels (IN1 = forward, IN2 = reverse).
//  M1, M2 additionally have a quadrature encoder input on ESP32 GPIOs.
// ---------------------------------------------------------------------------
#define CH_NUM_MOTORS       4

// {IN1 channel, IN2 channel}
#define CH_M1_IN1           0
#define CH_M1_IN2           1
#define CH_M2_IN1           2
#define CH_M2_IN2           3
#define CH_M3_IN1           15
#define CH_M3_IN2           14
#define CH_M4_IN1           13
#define CH_M4_IN2           12


// ---------------------------------------------------------------------------
//  Servos - direct ESP32 LEDC PWM
// ---------------------------------------------------------------------------
#define CH_NUM_SERVOS       4
#define CH_SV1_PIN          11
#define CH_SV2_PIN          12
#define CH_SV3_PIN          38
#define CH_SV4_PIN          39
#define CH_SERVO_FREQ       50      // Hz
#define CH_SERVO_MIN_US     500
#define CH_SERVO_MAX_US     2500

// ---------------------------------------------------------------------------
//  Multi-function ports
//  MF4P : two signal lines each (I2C1 / UART2 / dual GPIO)
//  MF3P : one signal line each  (GPIO / ADC / PWM / 1-Wire)
// ---------------------------------------------------------------------------
#define CH_NUM_MF4P         2
#define CH_NUM_MF3P         3

#define CH_MF4P_1_SIG1      16
#define CH_MF4P_1_SIG2      17
#define CH_MF4P_2_SIG1      18
#define CH_MF4P_2_SIG2      8

#define CH_MF3P_1_SIG       3
#define CH_MF3P_2_SIG       9
#define CH_MF3P_3_SIG       10




// ---------------------------------------------------------------------------
//  WS2812 / NeoPixel LED strip
// ---------------------------------------------------------------------------
#define CH_LED_STATUS_PIN 40
#define CH_LED_STATUS_NUM 12

// ---------------------------------------------------------------------------
//  Gamepad (BLE) - treat as disconnected if no frame within this window
// ---------------------------------------------------------------------------
#ifndef CH_GAMEPAD_TIMEOUT_MS
#define CH_GAMEPAD_TIMEOUT_MS  600
#endif

// ---------------------------------------------------------------------------
//  Phone Sensor (BLE) - treat as disconnected if no frame within this window
// ---------------------------------------------------------------------------
#ifndef CH_PHONE_SENSOR_TIMEOUT_MS
#define CH_PHONE_SENSOR_TIMEOUT_MS  600
#endif

// ---------------------------------------------------------------------------
//  RS485_1  (Smart Module bus - uses UART0 (Serial0) TXD0/RXD0 with DE/RE)
// ---------------------------------------------------------------------------
#define CH_RS485_DE_RE         2
#ifndef CH_RS485_BAUD
#define CH_RS485_BAUD       115200
#endif

// Which HardwareSerial the RS485 driver uses. Defaults to Serial0 (UART0),
// fixed at GPIO43 (TX0) / GPIO44 (RX0) on ESP32-S3. Serial0 is always a
// separate object from the native USB CDC (CH_USB_SERIAL below), so the two
// run concurrently with no pin conflict. Override by defining CH_RS485_SERIAL
// before including <ControlHub.h> - avoid Serial1, whose ESP32-S3 default
// pins (RX1=GPIO15/TX1=GPIO16) collide with CH_MF4P_1_SIG1.
#ifndef CH_RS485_SERIAL
#define CH_RS485_SERIAL     Serial0
#endif



// ---------------------------------------------------------------------------
//  RS485_2  (Smart Module bus - uses UART1 (Serial1) TXD1/RXD1 with DE/RE)
// ---------------------------------------------------------------------------
#define CH_RS485_2_DE_RE 6           
#ifndef CH_RS485_2_BAUD
#define CH_RS485_2_BAUD       115200
#endif

// pins (RX1=GPIO5/TX1=GPIO4)
#ifndef CH_RS485_2_SERIAL
#define CH_RS485_2_SERIAL     Serial1
#endif

// ---------------------------------------------------------------------------
//  USB Serial (host <-> hub)
//  On ESP32-S3, native USB CDC uses fixed GPIO20 (D+) / GPIO19 (D-).
//  These pins are hardware-fixed (not configurable) - do not reassign them
//  to any other peripheral in this pin map.
//  Requires the "USB CDC On Boot" board option to be enabled; otherwise
//  Serial falls back to UART0, colliding with CH_RS485_SERIAL above.
// ---------------------------------------------------------------------------
#ifndef CH_USB_SERIAL
#define CH_USB_SERIAL       Serial
#endif
#ifndef CH_USB_BAUD
#define CH_USB_BAUD         115200
#endif

// ---------------------------------------------------------------------------
//  BLE (Nordic UART Service)
// ---------------------------------------------------------------------------
#ifndef CH_BLE_DEFAULT_NAME
#define CH_BLE_DEFAULT_NAME "ControlHub"
#endif
#define CH_BLE_NUS_SERVICE_UUID     "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CH_BLE_NUS_RX_CHAR_UUID     "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // host -> hub
#define CH_BLE_NUS_TX_CHAR_UUID     "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // hub  -> host

// ---------------------------------------------------------------------------
//  Frame protocol
// ---------------------------------------------------------------------------
#define CH_FRAME_HEADER     0x55
#define CH_FRAME_MAX_DATA   8
#define CH_FRAME_TOTAL_MAX  (1 + 5 + CH_FRAME_MAX_DATA + 1)   // 15 bytes

#endif // CONTROLHUB_CONFIG_H
