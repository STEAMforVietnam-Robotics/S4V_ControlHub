// ============================================================================
//  ch_types.h — Public enums and type aliases
// ============================================================================
#pragma once

#include <stdint.h>

// ----- Motor identifier (1..6) -----
enum MotorId : uint8_t {
    MOTOR_1 = 0,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4,
    MOTOR_5,
    MOTOR_6,
    MOTOR_COUNT_ = 6      // sentinel
};

// ----- Servo identifier (1..4) -----
enum ServoId : uint8_t {
    SERVO_1 = 0,
    SERVO_2,
    SERVO_3,
    SERVO_4,
    SERVO_COUNT_ = 4
};

// ----- Multi-function port identifier -----
enum SensorPort : uint8_t {
    PORT_MF4P_1 = 0,    // 4-pin: SIG1+SIG2 (e.g. HC-SR04 trig+echo)
    PORT_MF4P_2,
    PORT_MF3P_1,        // 3-pin: single SIG
    PORT_MF3P_2,
    PORT_MF3P_3
};

// ----- Motor control mode (Level-3 API) -----
enum ControlMode : uint8_t {
    MODE_OPEN_LOOP = 0,  // raw PWM, no feedback
    MODE_SPEED,          // PID on RPM (needs encoder)
    MODE_POSITION        // PID on position in degrees (needs encoder)
};

// ----- Motor stopping behaviour -----
enum StopMode : uint8_t {
    STOP_COAST = 0,      // both H-bridge legs off, motor coasts
    STOP_BRAKE           // both legs low, motor brakes
};

// ----- Direction (used in raw calls) -----
enum Direction : int8_t {
    DIR_BACKWARD = -1,
    DIR_STOP     =  0,
    DIR_FORWARD  =  1
};

// ----- Sensor status -----
enum SensorStatus : uint8_t {
    SENSOR_OK = 0,
    SENSOR_NOT_INITIALIZED,
    SENSOR_OUT_OF_RANGE,
    SENSOR_TIMEOUT
};

// ============================================================================
//  Device-type tags for create() overload dispatch
//
//  Each tag is a distinct type so the compiler can pick the right create()
//  overload from the first argument.
//
//  Usage:   auto sonar = create(ULTRASONIC, PORT_MF4P_1);
// ============================================================================
namespace ch_tag {
    struct Ultrasonic_t {};
    struct ToF_t        {};
    struct SharpIR_t    {};
    struct Contact_t    {};
    struct Pot_t        {};
    struct NeoPixel_t   {};
    struct Buzzer_t     {};
    struct DigitalOut_t {};
}
// `static constexpr` gives each TU its own instance — works in C++11 too.
// Empty tag types have no observable state so distinct addresses are harmless.
static constexpr ch_tag::Ultrasonic_t ULTRASONIC{};
static constexpr ch_tag::ToF_t        TOF{};
static constexpr ch_tag::SharpIR_t    SHARP_IR{};
static constexpr ch_tag::Contact_t    CONTACT{};
static constexpr ch_tag::Pot_t        POT{};
static constexpr ch_tag::NeoPixel_t   NEOPIXEL{};
static constexpr ch_tag::Buzzer_t     BUZZER{};
static constexpr ch_tag::DigitalOut_t DIGITAL_OUT{};

// ----- Device kind (used internally by PortManager for diagnostics) -----
enum DeviceKind : uint8_t {
    DEV_NONE = 0,
    DEV_ULTRASONIC,
    DEV_TOF,
    DEV_SHARP_IR,
    DEV_CONTACT,
    DEV_POT,
    DEV_NEOPIXEL,
    DEV_BUZZER,
    DEV_DIGITAL_OUT,
};

// Slot id used by handle objects. 0xFF = invalid.
constexpr uint8_t INVALID_SLOT = 0xFF;
