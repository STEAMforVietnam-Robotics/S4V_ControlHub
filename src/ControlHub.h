// ============================================================================
//  ControlHub.h — single-include entry point
//
//      #include <ControlHub.h>
//
//      // Declare typed handles as globals (empty). Each create() takes a port
//      // and returns a handle owned by an internal registry.
//      UltrasonicHandle sonar;
//      ContactHandle    bumper;
//
//      void setup() {
//          Serial.begin(115200);
//          ControlHub.begin();
//
//          // create() AFTER begin(): hardware is up, so any port error prints
//          // to Serial right away.
//          sonar  = create(ULTRASONIC, PORT_MF4P_1);
//          bumper = create(CONTACT,    PORT_MF3P_1);
//          robot.set_primary_distance(sonar);
//          robot.set_primary_contact(bumper);
//      }
//      void loop() {
//          if (robot.is_obstacle_detected()) robot.stop();
//          else robot.go_forward();
//      }
// ============================================================================
#pragma once

#include <Adafruit_NeoPixel.h>

#include "ch_config.h"
#include "ch_types.h"
#include "PortManager.h"

// Core hardware (always present)
#include "devices/Motor.h"
#include "devices/Servo.h"

// Pluggable devices (instantiate via create())
#include "devices/Ultrasonic.h"
#include "devices/ToF.h"
#include "devices/SharpIR.h"
#include "devices/Contact.h"
#include "devices/Pot.h"
#include "devices/DigitalOut.h"
#include "devices/NeoPixel.h"
#include "devices/Buzzer.h"

// APIs
#include "api/SysAPI.h"
#include "api/RobotAPI.h"
#include "api/Create.h"

typedef enum {
    LED_RED     = 0,
    LED_BLUE    = 1,
    LED_GREEN   = 2,
    LED_YELLOW  = 3,
    LED_CYAN    = 4,
    LED_MAGENTA = 5,
    LED_WHITE   = 6,
    LED_OFF     = 7
} LedColor;


class _ControlHub {
public:
    void begin();
    bool isReady() const { return _ready; }
    void setLedColor(uint8_t r, uint8_t g, uint8_t b) { ledBoard->fill(r<<16 | g<<8 | b); ledBoard->show(); }
    void setLedBrightness(uint8_t b) { ledBoard->setBrightness(b);}
    void setLedColor(LedColor color) {
        switch (color) {
            case LED_RED:     setLedColor(255, 0, 0); break;
            case LED_BLUE:    setLedColor(0, 0, 255); break;
            case LED_GREEN:   setLedColor(0, 255, 0); break;
            case LED_YELLOW:  setLedColor(255, 255, 0); break;
            case LED_CYAN:    setLedColor(0, 255, 255); break;
            case LED_MAGENTA: setLedColor(255, 0, 255); break;
            case LED_WHITE:   setLedColor(255, 255, 255); break;
            case LED_OFF:     setLedColor(0, 0, 0); break;
        }
    }
private:
    bool _ready = false;
    Adafruit_NeoPixel* ledBoard;
};

extern _ControlHub ControlHub;
