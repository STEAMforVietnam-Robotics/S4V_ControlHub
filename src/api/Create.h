// ============================================================================
//  api/Create.h — Single entry point `create(TAG, port, ...)`
//
//  Each overload picks the right driver, instantiates it inside the registry,
//  and returns a typed handle. The handle is invalid (.valid()==false) if
//  creation failed — typically port conflict or capability mismatch.
//
//  Usage — declare handles globally (empty), then create() inside setup()
//  after ControlHub.begin():
//      UltrasonicHandle sonar;
//      NeoPixelHandle   leds;
//      void setup() {
//          Serial.begin(115200);
//          ControlHub.begin();
//          sonar = create(ULTRASONIC, PORT_MF4P_1);
//          leds  = create(NEOPIXEL,   PORT_MF3P_1, /*num_pixels=*/8);
//      }
//
//  Other tags: create(TOF, port[, addr]), create(SHARP_IR, port),
//  create(CONTACT, port[, active_high]), create(POT, port),
//  create(BUZZER, port), create(DIGITAL_OUT, port[, initial]).
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../devices/Ultrasonic.h"
#include "../devices/ToF.h"
#include "../devices/SharpIR.h"
#include "../devices/Contact.h"
#include "../devices/Pot.h"
#include "../devices/DigitalOut.h"
#include "../devices/NeoPixel.h"
#include "../devices/Buzzer.h"

// Distance sensors
UltrasonicHandle  create(ch_tag::Ultrasonic_t,  SensorPort port);
ToFHandle         create(ch_tag::ToF_t,         SensorPort port, uint8_t i2c_addr = 0x29);
SharpIRHandle     create(ch_tag::SharpIR_t,     SensorPort port);

// Digital input / analog input
ContactHandle     create(ch_tag::Contact_t,     SensorPort port, bool active_high = false);
PotHandle         create(ch_tag::Pot_t,         SensorPort port);

// Outputs
DigitalOutHandle  create(ch_tag::DigitalOut_t,  SensorPort port, bool initial_state = false);
NeoPixelHandle    create(ch_tag::NeoPixel_t,    SensorPort port, uint16_t num_pixels);
BuzzerHandle      create(ch_tag::Buzzer_t,      SensorPort port);
