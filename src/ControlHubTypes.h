// ============================================================================
//  ch_types.h — Public enums and type aliases
// ============================================================================
#pragma once

#include <stdint.h>

// ----- Motor identifier (1..4) -----
// IDs are 1-based: MotorManager::_idx() maps id N → array index N-1, so MOTOR_1
// must equal 1 (not 0) for motor 1 to be addressable.
enum MotorPort : uint8_t {
    M1 = 1,
    M2,
    M3,
    M4,
    MOTOR_PORT_COUNT_ = 4      // sentinel (== number of motors)
};

// ----- Servo identifier (1..4) -----
// 1-based for the same reason as MotorPort (ServoManager::_idx() = id - 1).
enum ServoPort : uint8_t {
    S1 = 1,
    S2,
    S3,
    S4,
    SERVO_PORT_COUNT_ = 4
};

// ----- RS485 bus selector -----
// The board exposes two independent RS485 Smart Module buses.
enum Rs485Bus : uint8_t {
    RS485_1 = 0,
    RS485_2 = 1
};

// ----- Multi-function port identifier -----
enum Port : uint8_t {
    PORT_MF4P_1 = 0,    // 4-pin: SIG1+SIG2 (e.g. HC-SR04 trig+echo)
    PORT_MF4P_2,
    PORT_MF3P_1,        // 3-pin: single SIG
    PORT_MF3P_2,
    PORT_MF3P_3
};

// NOTE: Motor control modes live in protocol/Frame.h as `MotorMode`
// (MODE_DUTY / MODE_SPEED / MODE_POSITION) so the wire protocol and the API
// share one definition. Level-3 closed-loop (SPEED/POSITION) is not wired up
// yet; only MODE_DUTY (open loop) is currently honoured.

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
