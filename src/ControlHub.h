/*  ControlHub.h  --  Top-level include for the Control Hub library.
 *
 *  Usage sketch:
 *  -------------
 *      #include <ControlHub.h>
 *
 *      Motor      leftM (M1), rightM(M2);
 *      Servo      gripper(S1);
 *      Ultrasonic dSensor(P1);
 *      Contact    bumper (P4);
 *
 *      void setup() {
 *          ControlHub.begin();               // USB + BLE + RS485 all on
 *          leftM.begin();  rightM.begin();   // modules self-register in their
 *          gripper.begin();  dSensor.begin();//   constructors - no attach()
 *          bumper.begin();
 *          Robot.setDriveTrain(&leftM, &rightM);
 *          Robot.setGripper (&gripper);
 *          Robot.setDistance(&dSensor, 200);
 *          Robot.setContact (&bumper);
 *      }
 *
 *      void loop() {
 *          ControlHub.update();              // <-- polls host + smart bus
 *
 *          if (Robot.is_obstacle_detected()) Robot.stop();
 *          else                              Robot.go_forward(60);
 *      }
 */

#ifndef CONTROLHUB_H
#define CONTROLHUB_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "ControlHubConfig.h"

// Protocol layer
#include "protocol/CRC8.h"
#include "protocol/Frame.h"

// Transports
#include "transport/Transport.h"
#include "transport/UsbTransport.h"
#include "transport/BleTransport.h"
#include "transport/Rs485Transport.h"

// Core
#include "core/CommHub.h"

// Modules
#include "modules/Module.h"
#include "modules/Motor.h"
#include "modules/Servo.h"
#include "modules/Ultrasonic.h"
#include "modules/SmartToF.h"
#include "modules/SmartBumper.h"
#include "modules/Contact.h"
#include "modules/Led.h"
#include "modules/Gamepad.h"
#include "modules/PhoneSensor.h"

// High-level Robot / System APIs
#include "robot/Robot.h"
#include "system/System.h"

// Convenience re-export of the protocol namespace at global scope so
// beginner sketches can write `MOD_MOTOR`, `MODE_DUTY`, `Frame`, etc.
using ControlHubProtocol::Frame;
using ControlHubProtocol::FrameParser;
using ControlHubProtocol::ModuleType;
using ControlHubProtocol::MotorMode;
using ControlHubProtocol::MOD_POWER;
using ControlHubProtocol::MOD_IMU;
using ControlHubProtocol::MOD_TOF;
using ControlHubProtocol::MOD_BUMPER;
using ControlHubProtocol::MOD_MOTOR;
using ControlHubProtocol::MOD_SERVO;
using ControlHubProtocol::MOD_LED;
using ControlHubProtocol::MOD_GAMEPAD;
using ControlHubProtocol::MOD_PHONE_SENSOR;
using ControlHubProtocol::MODE_DUTY;
using ControlHubProtocol::MODE_SPEED;
using ControlHubProtocol::MODE_POSITION;

// ---------------------------------------------------------------------------
//  ControlHub
// ---------------------------------------------------------------------------
class _ControlHub {
public:
    _ControlHub();

    // ---- Lifecycle -----------------------------------------------------
    /** Start everything with defaults (USB + BLE + RS485). */
    void begin();

    /** Fine-grained variant: turn each transport on/off individually. */
    void begin(bool enableUsb, bool enableBle, bool enableRs485,
               const char *bleName = CH_BLE_DEFAULT_NAME);

    /** Start only the USB transport. */
    void beginUSB   (unsigned long baud = CH_USB_BAUD);
    /** Start only the BLE transport, advertising as @p name. */
    void beginBLE   (const char *name  = CH_BLE_DEFAULT_NAME);
    /** Start only RS485 (Smart Module) bus #1. */
    void beginRS485 (unsigned long baud = CH_RS485_BAUD);
    /** Start only RS485 (Smart Module) bus #2. */
    void beginRS485_2(unsigned long baud = CH_RS485_2_BAUD);

    /**
     * Poll incoming frames, run closed-loop motor ticks, and dispatch
     * everything. Call this once per loop() iteration.
     */
    void update();

    // Note: on-board modules (Motor / Servo / Ultrasonic / Contact) register
    // themselves automatically in their constructor, so there is no attach().

    // ---- Access --------------------------------------------------------
    ControlHubProtocol::CommHub &comm()   { return _hub; }

    /** Send a frame to whichever host (USB / BLE) is currently attached. */
    void sendFrame  (const Frame &f) { _hub.sendFrame(f); }

    /** Send a frame down RS485 Smart Module bus #1. */
    void sendRs485  (const Frame &f) { _hub.sendRs485(f); }

    /** Send a frame down RS485 Smart Module bus #2. */
    void sendRs485_2(const Frame &f) { _hub.sendRs485_2(f); }

    /** Register a global handler for host-side frames (USB / BLE). */
    void onFrame(ControlHubProtocol::FrameHandler cb) { _hub.onFrame(cb); }

    // ---- On-board WS2812 status LED(s) --------------------------------
    // Fixed to CH_LED_STATUS_PIN / CH_LED_STATUS_NUM. Auto-initialised on
    // the first call, so you can use them right after ControlHub.begin().

    /** All status LEDs to one colour and refresh. */
    void statusColor(uint8_t r, uint8_t g, uint8_t b);
    /** A single status LED (0-based) and refresh. */
    void statusPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b);
    /** Overall brightness 0..255 (default 64 to avoid glare). */
    void statusBrightness(uint8_t b);
    /** Toggle the whole strip between (r,g,b) and off on each call. */
    void statusBlink(uint8_t r, uint8_t g, uint8_t b);
    /** Turn every status LED off. */
    void statusOff();

    // Convenience states
    void statusOk()      { statusColor(0,   255, 0);   }   // green
    void statusError()   { statusColor(255, 0,   0);   }   // red
    void statusWarning() { statusColor(255, 128, 0);   }   // amber
    void statusBusy()    { statusColor(0,   0,   255); }   // blue

private:
    void _installDefaultHandlers();
    void _route(const Frame &f, const char *src);
    void _initStatusLed();

    ControlHubProtocol::CommHub _hub;

    Adafruit_NeoPixel _statusLed{CH_LED_STATUS_NUM, CH_LED_STATUS_PIN,
                                 NEO_GRB + NEO_KHZ800};
    uint8_t _statusBrightness = 64;
    bool    _statusBegun      = false;
    bool    _statusBlinkOn    = false;
};

extern _ControlHub ControlHub;

#endif // CONTROLHUB_H
