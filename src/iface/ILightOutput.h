#pragma once

#include <stdint.h>
#include "IDevice.h"

class ILightOutput : public IDevice {
public:
    virtual ~ILightOutput() = default;
    // Number of independently addressable pixels (1 for a single LED).
    virtual uint16_t count() = 0;
    // Set pixel `i` to RGB (0..255 each). Doesn't transmit; call show().
    virtual void set_color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void fill(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void clear() = 0;
    // Push the pixel buffer to the hardware.
    virtual void show() = 0;
};
