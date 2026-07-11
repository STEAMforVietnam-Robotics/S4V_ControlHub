/*  Led.h
 *
 *  WS2812 / NeoPixel LED strip on a multi-function port (single data line).
 *  Uses the Adafruit NeoPixel library for the RMT bit-banging.
 *
 *      L1/L2 API:
 *          Led strip(PORT_MF3P_1, 8);
 *          strip.begin();
 *          strip.setColor(0, 0, 255);       // all LEDs blue
 *          strip.setPixel(0, 255, 0, 0);    // first LED red
 *          strip.setBrightness(128);
 *          strip.clear();
 *
 *  Frame protocol: MOD_LED
 *      MOTOR_MSG_CONTROL, data = [index, r, g, b, (brightness)]
 *          index == 0xFF -> all LEDs
 *      RTR -> status: [count_hi, count_lo, brightness, fault]
 */

#ifndef CONTROLHUB_LED_H
#define CONTROLHUB_LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../ControlHubConfig.h"
#include "../ControlHubTypes.h"
#include "../protocol/Frame.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

class Led : public Module {
public:
    /** @param port data line (MF3P single SIG, or MF4P SIG1).
     *  @param count number of WS2812 LEDs in the strip.
     *  @param id led ID. */
    explicit Led(Port port = PORT_MF3P_1, uint16_t count = 8, uint8_t id = 0);

    void begin() override;

    /** Set every LED to the same colour and refresh. */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    /** Set a single LED (0-based) and refresh. */
    void setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b);

    /** Fill the whole strip in the buffer WITHOUT refreshing (call show()). */
    void fill(uint8_t r, uint8_t g, uint8_t b);

    /** Overall brightness 0..255. */
    void setBrightness(uint8_t b);

    /** Turn all LEDs off and refresh. */
    void clear();

    /** Push the buffer to the strip. */
    void show();

    uint16_t count() const { return _count; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_LED; }
    uint8_t id()         const override { return _id; }

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

    ControlHubProtocol::Frame makeStatusFrame() const;

private:
    Port     _port;
    uint8_t  _id;                   // wire ID: 1-based port index
    int      _pin       = -1;
    uint16_t _count;
    uint8_t  _brightness = 255;
    bool     _begun     = false;
    uint8_t  _fault     = 0;
    Adafruit_NeoPixel _px;
};

#endif
