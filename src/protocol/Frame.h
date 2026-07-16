/*  Frame.h  --  Wire-format frame used on RS485, USB Serial and BLE.
 *
 *      +--------+-----------+-----------+--------+-----+-----+----------+------+
 *      | Header | ModuleType| ModuleID  | MsgID  | RTR | DLC | Data...  | CRC8 |
 *      |  0x55  |   1 B     |   1 B     |  1 B   | 1 B | 1 B | 0..8 B   | 1 B  |
 *      +--------+-----------+-----------+--------+-----+-----+----------+------+
 *
 *  Layout, semantics and CRC-8 algorithm are taken from RVRS485060726162210.pdf.
 */

#ifndef CONTROLHUB_FRAME_H
#define CONTROLHUB_FRAME_H

#include <Arduino.h>
#include "../ControlHubConfig.h"

namespace ControlHubProtocol {

// ---- Module Type ---------------------------------------------------------
enum ModuleType : uint8_t {
    MOD_SYSTEM  = 0x00,   // Local extension - Control Hub itself / system-level
    MOD_POWER   = 0x01,   // Power Management
    MOD_IMU     = 0x02,   // IMU Sensor
    MOD_TOF     = 0x03,   // ToF distance sensor
    MOD_BUMPER  = 0x04,   // Bumper / contact sensor
    MOD_MOTOR      = 0x05,   // Motor Driver
    MOD_SERVO      = 0x06,   // Local extension - on-board servo channels
    MOD_ULTRASONIC = 0x07,   // Ultrasonic distance sensor
    MOD_SMART_MOTOR = 0x08,  // Smart Motor (with integrated encoder)
    MOD_LED         = 0x09,  // WS2812 / NeoPixel LED strip
    MOD_GAMEPAD     = 0x0A,   // BLE gamepad (buttons + joysticks)
    MOD_PHONE_SENSOR = 0x0B   // Smart Phone Sensor (Accel, Gyro, Mag, GPS)
};

// ---- Msg IDs -------------------------------------------------------------
// Motor Driver
enum MotorMsgId : uint8_t {
    MSG_CONTROL = 0x01,   // set duty / speed / position
    MSG_CONFIG  = 0x02,   // set ID (or other config)
    MSG_STATUS  = 0x11    // feedback (RTR or async); sensors & power reuse this
};

// Phone Sensor - dữ liệu đẩy từ app điện thoại qua BLE (push-only, không RTR)
enum PhoneMsgId : uint8_t {
    PHONE_MSG_ACCEL   = 0x11,   // 3x int16 big-endian, đơn vị mg (milli-g), scale 1000 = 1g
    PHONE_MSG_GYRO    = 0x12,   // 3x int16 big-endian, đơn vị 0.1 deg/s
    PHONE_MSG_MAG     = 0x13,   // 3x int16 big-endian, đơn vị 0.1 µT
    PHONE_MSG_GPS     = 0x14,   // lat (int32) + lon (int32), độ * 1e7
    PHONE_MSG_GPS_ALT = 0x15    // alt (int16), đơn vị decimet (0.1 m)
};

// ---- Motor control modes (byte[2] of a Control Frame) -------------------
enum MotorMode : uint8_t {
    MODE_DUTY     = 0x00,   // raw duty cycle
    MODE_SPEED    = 0x01,   // closed-loop speed (rpm)
    MODE_POSITION = 0x02    // closed-loop position (encoder ticks)
};

// ---- Frame ---------------------------------------------------------------
struct Frame {
    uint8_t moduleType = 0;
    uint8_t moduleId   = 0;
    uint8_t msgId      = 0;
    uint8_t rtr        = 0;
    uint8_t dlc        = 0;
    uint8_t data[CH_FRAME_MAX_DATA] = {0};
    uint8_t crc        = 0;         // filled by serialize()

    /**
     * Serialize into a byte buffer.
     * @param out buffer of at least CH_FRAME_TOTAL_MAX bytes
     * @return    number of bytes written (7 .. 15)
     */
    size_t  serialize(uint8_t *out) const;

    /** Recompute CRC over the header + payload (does not touch @c crc). */
    uint8_t computeCrc() const;

    // Convenience helpers used by the module classes ------------------
    static Frame makeMotorControl(uint8_t id, int16_t dutyPercentX100, MotorMode mode);
    static Frame makeMotorConfig (uint8_t currentId, uint8_t newId);
    static Frame makeStatusRequest(uint8_t moduleType, uint8_t moduleId,
                                   uint8_t msgId = MSG_STATUS);
};

// ---- Stream parser -------------------------------------------------------
/**
 * Byte-at-a-time parser suitable for UART / BLE streams. Feed every incoming
 * byte to @ref feed(); when a complete, CRC-valid frame arrives the method
 * returns true and writes it to @p out. Malformed frames are silently
 * dropped and the parser resynchronizes on the next 0x55 header.
 */
class FrameParser {
public:
    /** Push one byte from the stream. */
    bool feed(uint8_t byte, Frame &out);

    /** Reset internal state (call on disconnect). */
    void reset();

private:
    enum State : uint8_t {
        S_HEADER, S_MODTYPE, S_MODID, S_MSGID,
        S_RTR, S_DLC, S_DATA, S_CRC
    } _state = S_HEADER;

    Frame   _work;
    uint8_t _idx = 0;
};

} // namespace ControlHubProtocol

#endif
