// ============================================================================
//  ch_config.h — Hardware pin mapping for Control Hub (ESP32-S3)
//  Generated from README.md section 3–4.
//  Edit here ONLY when the PCB hardware mapping changes.
// ============================================================================
#pragma once

#include "ch_types.h"   // enums referenced by the macros below

// ----- I2C bus 0 (Wire) — internal bus, PCA9685 -----
#define CH_I2C_SDA              13
#define CH_I2C_SCL              12
#define CH_I2C_FREQ_HZ          400000UL

// ----- I2C bus 1 (Wire1) — multi-function 4P ports -----
// PCA9685 stays on Wire (I2C0). I2C sensors cabled to a 4P port use Wire1,
// brought up lazily on whichever MF4P port the first I2C device claims.
// On that host port: SIG1 = SDA, SIG2 = SCL.
#define CH_I2C1_FREQ_HZ         400000UL
#define CH_MAX_I2C_DEVICES      4

// ----- PCA9685 -----
#define CH_PCA9685_ADDR         0x40
#define CH_PCA9685_PWM_FREQ_HZ  1500   // ~1.5 kHz for DC motor PWM via DRV8874

// ----- DC Motor → PCA9685 channel mapping (IN1, IN2) -----
// IN1, IN2 drive the two halves of an H-bridge on the DRV8874.
// Mapping copied verbatim from README section 4.2–4.3.
#define CH_M1_IN1_CH    4
#define CH_M1_IN2_CH    5
#define CH_M2_IN1_CH    11
#define CH_M2_IN2_CH    10
#define CH_M3_IN1_CH    0
#define CH_M3_IN2_CH    1
#define CH_M4_IN1_CH    2
#define CH_M4_IN2_CH    3
#define CH_M5_IN1_CH    15
#define CH_M5_IN2_CH    14
#define CH_M6_IN1_CH    13
#define CH_M6_IN2_CH    12

// ----- Encoder GPIO (only M1, M2 have encoders) -----
#define CH_M1_ENC_A     7
#define CH_M1_ENC_B     6
#define CH_M2_ENC_A     47
#define CH_M2_ENC_B     21

// ----- Servo GPIO -----
#define CH_SV1_PIN      41
#define CH_SV2_PIN      40
#define CH_SV3_PIN      38
#define CH_SV4_PIN      39

// ----- Multi-function port GPIO -----
// 4P ports: two signal lines per port (SIG1, SIG2)
#define CH_MF4P_1_SIG1  16
#define CH_MF4P_1_SIG2  17
#define CH_MF4P_2_SIG1  18
#define CH_MF4P_2_SIG2  8    // NOT 19: GPIO19 is reserved for native USB D-
// 3P ports: one signal line per port (SIG)
#define CH_MF3P_1_SIG   9
#define CH_MF3P_2_SIG   10
#define CH_MF3P_3_SIG   11

// ----- Board LEDs WS2812 -----
#define CH_LED_PIN  3
#define LED_BOARD_NUM 12

// ----- User buttons -----
#define CH_BOOT_PIN 0

// ----- USB Serial (for debugging) -----
#define CH_USB_P 20
#define CH_USB_N 19


// ============================================================================
//  Defaults for Level-1 (robot.*) high-level behaviour
//  Change these to retarget the default sensors/actuators.
// ============================================================================

// Drivetrain: 2-wheel with encoder motors (M1 left, M2 right)
#define CH_DRIVE_LEFT_MOTOR     MOTOR_1
#define CH_DRIVE_RIGHT_MOTOR    MOTOR_2

// Gripper servo for robot.grasp() / robot.release()
#define CH_GRIPPER_SERVO        SERVO_1
#define CH_GRIPPER_OPEN_DEG     20      // angle when released
#define CH_GRIPPER_CLOSE_DEG    140     // angle when grasping

// Default obstacle threshold for robot.is_obstacle_detected().
#define CH_OBSTACLE_THRESHOLD_CM    15

// NOTE: there are no default ports for distance/contact sensors any more.
// Users declare them explicitly via create(...) and register with
// robot.set_primary_distance() / set_primary_contact().

// ----- Speed defaults -----
#define CH_DEFAULT_DRIVE_SPEED      60     // % for robot.go_forward(), etc.

// ----- PID defaults (per motor, can be overridden at runtime) -----
#define CH_DEFAULT_KP    1.20f
#define CH_DEFAULT_KI    0.05f
#define CH_DEFAULT_KD    0.00f

// ----- Encoder -----
// Counts per revolution of the OUTPUT shaft. Adjust to your motor+gearbox.
// Example: 11 PPR encoder × 4 (quadrature) × 30:1 gearbox = 1320 CPR
#define CH_M1_CPR        1320
#define CH_M2_CPR        1320

// ----- PID loop -----
#define CH_PID_TASK_HZ         100      // 100 Hz update rate
#define CH_PID_TASK_STACK      4096
#define CH_PID_TASK_PRIORITY   2
#define CH_PID_TASK_CORE       1        // pin to core 1, free core 0 for app
