#include "Encoder.h"

// ---------------------------------------------------------------------------
//  Static slot table — one entry per attached encoder. Kept in IRAM so the
//  ISR doesn't need to fetch it from flash.
// ---------------------------------------------------------------------------
struct EncSlot {
    uint8_t  pinA;
    uint8_t  pinB;
    volatile int32_t count;
    volatile uint8_t prev;     // packed prev state (A<<1|B)
    bool     in_use;
};

static EncSlot s_slots[Encoder::MAX_ENCODERS] = {};

// x4 quadrature transition table.
// Indexed by (prev << 2 | curr) where state = A<<1|B
// Returns delta: +1, -1, or 0 (invalid/no change)
static const int8_t kQuadTable[16] = {
    0, -1, +1,  0,
   +1,  0,  0, -1,
   -1,  0,  0, +1,
    0, +1, -1,  0
};

// ---------------------------------------------------------------------------
//  ISR (one per slot)
// ---------------------------------------------------------------------------
template <uint8_t N>
static void IRAM_ATTR enc_isr() {
    EncSlot& s = s_slots[N];
    uint8_t a = (uint8_t)digitalRead(s.pinA);
    uint8_t b = (uint8_t)digitalRead(s.pinB);
    uint8_t curr = (a << 1) | b;
    uint8_t idx = (s.prev << 2) | curr;
    s.count += kQuadTable[idx];
    s.prev = curr;
}

using IsrFn = void(*)();
static const IsrFn kIsrTable[Encoder::MAX_ENCODERS] = {
    enc_isr<0>, enc_isr<1>, enc_isr<2>, enc_isr<3>
};

// ---------------------------------------------------------------------------
Encoder::Encoder() : _slot(-1), _cpr(1), _last_count_for_rpm(0) {}

bool Encoder::attach(uint8_t pinA, uint8_t pinB, int32_t cpr) {
    // Find a free slot.
    int8_t free = -1;
    for (uint8_t i = 0; i < MAX_ENCODERS; ++i) {
        if (!s_slots[i].in_use) { free = i; break; }
    }
    if (free < 0) return false;

    EncSlot& s = s_slots[free];
    s.pinA = pinA;
    s.pinB = pinB;
    s.count = 0;
    s.in_use = true;

    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    uint8_t a = (uint8_t)digitalRead(pinA);
    uint8_t b = (uint8_t)digitalRead(pinB);
    s.prev = (a << 1) | b;

    attachInterrupt(digitalPinToInterrupt(pinA), kIsrTable[free], CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), kIsrTable[free], CHANGE);

    _slot = free;
    _cpr  = (cpr > 0) ? cpr : 1;
    return true;
}

int32_t Encoder::count() const {
    if (_slot < 0) return 0;
    noInterrupts();
    int32_t c = s_slots[_slot].count;
    interrupts();
    return c;
}

float Encoder::positionDeg() const {
    if (_slot < 0) return 0.0f;
    return (float)count() * 360.0f / (float)_cpr;
}

float Encoder::updateRpm(float dt_s) {
    if (_slot < 0 || dt_s <= 0.0f) return 0.0f;
    int32_t c = count();
    int32_t delta = c - _last_count_for_rpm;
    _last_count_for_rpm = c;
    // rpm = (delta / cpr) revolutions / dt seconds * 60
    return ((float)delta / (float)_cpr) * (60.0f / dt_s);
}

void Encoder::reset(int32_t value) {
    if (_slot < 0) return;
    noInterrupts();
    s_slots[_slot].count = value;
    interrupts();
    _last_count_for_rpm = value;
}
