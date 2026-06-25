#include "Buzzer.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

// ESP32-S3 has 16 LEDC channels in total. ESP32Servo's allocateTimer()
// reserves timers 0-3. We reserve channels 8..15 for buzzer/aux use.
namespace {
    static constexpr int8_t LEDC_BUZZER_BASE = 8;
    static constexpr int8_t LEDC_BUZZER_MAX  = 16;   // exclusive upper
    static bool s_chan_used[LEDC_BUZZER_MAX - LEDC_BUZZER_BASE] = {};
}

int8_t Buzzer::_alloc_channel() {
    for (int i = 0; i < LEDC_BUZZER_MAX - LEDC_BUZZER_BASE; ++i) {
        if (!s_chan_used[i]) { s_chan_used[i] = true; return LEDC_BUZZER_BASE + i; }
    }
    return -1;
}
void Buzzer::_free_channel(int8_t ch) {
    int i = ch - LEDC_BUZZER_BASE;
    if (i >= 0 && i < LEDC_BUZZER_MAX - LEDC_BUZZER_BASE) s_chan_used[i] = false;
}

Buzzer::Buzzer(SensorPort port)
    : _port(port), _pin(0xFF), _ledc_channel(-1), _ok(false)
{
    PortManager::claim(port, DEV_BUZZER, CAP_PWM | CAP_GPIO_DIGITAL);
}

bool Buzzer::hwInit() {
    _pin = PortManager::pin(_port, 0);
    if (_pin == 0xFF) return false;
    _ledc_channel = _alloc_channel();
    if (_ledc_channel < 0) {
        Serial.println("[ControlHub] Buzzer: out of LEDC channels");
        return false;
    }
    // Arduino-ESP32 v3.x: ledcAttach(pin, freq, resolution_bits)
    ledcAttach(_pin, 1000, 8);
    ledcWrite(_pin, 0);                 // start silent
    _ok = true;
    return true;
}

Buzzer::~Buzzer() {
    if (_ok) {
        ledcWrite(_pin, 0);
        ledcDetach(_pin);
        _free_channel(_ledc_channel);
        PortManager::release(_port);
    }
}

void Buzzer::tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (!_ok || freq_hz == 0) return;
    ledcWriteTone(_pin, freq_hz);
    if (duration_ms > 0) {
        delay(duration_ms);
        no_tone();
    }
}

void Buzzer::no_tone() {
    if (_ok) ledcWriteTone(_pin, 0);
}

// --- handle proxy ---
static Buzzer* _asBuzzer(uint8_t slot) {
    DeviceSlot* s = DeviceRegistry::get(slot);
    return (s && s->kind == DEV_BUZZER) ? static_cast<Buzzer*>(s->impl) : nullptr;
}
void BuzzerHandle::tone(uint32_t f, uint32_t ms) const {
    Buzzer* b=_asBuzzer(_slot); if (b) b->tone(f, ms);
}
void BuzzerHandle::no_tone() const {
    Buzzer* b=_asBuzzer(_slot); if (b) b->no_tone();
}
