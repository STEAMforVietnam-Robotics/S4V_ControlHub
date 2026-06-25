#include "NeoPixel.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>



NeoPixel::NeoPixel(SensorPort port, uint16_t num_pixels)
    : _port(port), _num(num_pixels), _ok(false), _strip(nullptr)
{
    PortManager::claim(port, DEV_NEOPIXEL, CAP_RMT | CAP_GPIO_DIGITAL);
}

bool NeoPixel::hwInit() {
    uint8_t pin = PortManager::pin(_port, 0);
    if (pin == 0xFF) return false;
    _strip = new Adafruit_NeoPixel(_num, pin, NEO_GRB + NEO_KHZ800);
    _strip->begin();
    _strip->clear();
    _strip->show();
    _ok = true;
    return true;
}

NeoPixel::~NeoPixel() {
    if (_strip) delete _strip;
    if (_ok) PortManager::release(_port);
}

uint16_t NeoPixel::count() {
    return _ok ? _strip->numPixels() : 0;
}

void NeoPixel::set_color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
    if (_ok) _strip->setPixelColor(i, r, g, b);
}

void NeoPixel::fill(uint8_t r, uint8_t g, uint8_t b) {
    if (!_ok) return;
    for (uint16_t i = 0; i < _strip->numPixels(); ++i)
        _strip->setPixelColor(i, r, g, b);
}

void NeoPixel::clear()             { if (_ok) _strip->clear(); }
void NeoPixel::show()              { if (_ok) _strip->show(); }
void NeoPixel::set_brightness(uint8_t b) { if (_ok) _strip->setBrightness(b); }

// --- handle proxy ---
static NeoPixel* _asNP(uint8_t slot) {
    DeviceSlot* s = DeviceRegistry::get(slot);
    return (s && s->kind == DEV_NEOPIXEL) ? static_cast<NeoPixel*>(s->impl) : nullptr;
}

uint16_t NeoPixelHandle::count() const { NeoPixel* n=_asNP(_slot); return n ? n->count() : 0; }
void NeoPixelHandle::set_color(uint16_t i, uint8_t r, uint8_t g, uint8_t b) const {
    NeoPixel* n=_asNP(_slot); if (n) n->set_color(i, r, g, b);
}
void NeoPixelHandle::fill(uint8_t r, uint8_t g, uint8_t b) const {
    NeoPixel* n=_asNP(_slot); if (n) n->fill(r, g, b);
}
void NeoPixelHandle::clear() const          { NeoPixel* n=_asNP(_slot); if (n) n->clear(); }
void NeoPixelHandle::show()  const          { NeoPixel* n=_asNP(_slot); if (n) n->show(); }
void NeoPixelHandle::set_brightness(uint8_t b) const {
    NeoPixel* n=_asNP(_slot); if (n) n->set_brightness(b);
}
