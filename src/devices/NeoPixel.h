// ============================================================================
//  devices/NeoPixel.h — WS2812/WS2812B strip on any port with GPIO (uses RMT).
//
//  Uses Adafruit_NeoPixel under the hood. Pixel buffer lives in this driver.
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/ILightOutput.h"

class Adafruit_NeoPixel;   // forward decl

class NeoPixel : public ILightOutput {
public:
    NeoPixel(SensorPort port, uint16_t num_pixels);
    ~NeoPixel() override;

    bool hwInit() override;
    uint16_t count() override;
    void set_color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) override;
    void fill(uint8_t r, uint8_t g, uint8_t b) override;
    void clear() override;
    void show() override;

    // Convenience: set brightness 0..255 (applied globally on next show()).
    void set_brightness(uint8_t b);

private:
    SensorPort         _port;
    uint16_t           _num;       // pixel count, applied at hwInit()
    bool               _ok;
    Adafruit_NeoPixel* _strip;
};

class NeoPixelHandle {
public:
    NeoPixelHandle() : _slot(INVALID_SLOT) {}
    explicit NeoPixelHandle(uint8_t slot) : _slot(slot) {}

    uint16_t count() const;
    void set_color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) const;
    void fill(uint8_t r, uint8_t g, uint8_t b) const;
    void clear() const;
    void show() const;
    void set_brightness(uint8_t b) const;

    bool valid() const { return _slot != INVALID_SLOT; }

private:
    uint8_t _slot;
};


