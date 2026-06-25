// ============================================================================
//  devices/Buzzer.h — Passive buzzer driven by ESP32 LEDC PWM.
//
//  Usage:
//      auto bz = create(BUZZER, PORT_MF3P_1);
//      bz.tone(440);          // play A4 forever
//      bz.tone(880, 500);     // play A5 for 500 ms (blocking), then stop
//      bz.no_tone();          // silence
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDevice.h"

class Buzzer : public IDevice {
public:
    explicit Buzzer(SensorPort port);
    ~Buzzer() override;

    bool hwInit() override;
    // Start tone at freq Hz. If duration_ms > 0, blocks then silences.
    void tone(uint32_t freq_hz, uint32_t duration_ms = 0);
    void no_tone();

private:
    SensorPort _port;
    uint8_t    _pin;
    int8_t     _ledc_channel;
    bool       _ok;

    static int8_t _alloc_channel();
    static void   _free_channel(int8_t ch);
};

class BuzzerHandle {
public:
    BuzzerHandle() : _slot(INVALID_SLOT) {}
    explicit BuzzerHandle(uint8_t slot) : _slot(slot) {}
    void tone(uint32_t freq_hz, uint32_t duration_ms = 0) const;
    void no_tone() const;
    bool valid() const { return _slot != INVALID_SLOT; }
private:
    uint8_t _slot;
};
