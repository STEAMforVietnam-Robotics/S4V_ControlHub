#include "I2CBus.h"
#include "../PortManager.h"
#include "../ch_config.h"
#include <Wire.h>

namespace {
    bool s_begun = false;     // Wire1.begin() called once
}

TwoWire* ch_i2c::bus1() {
    if (!PortManager::i2cBound()) return nullptr;

    if (!s_begun) {
        SensorPort p   = PortManager::i2cPort();
        uint8_t    sda = PortManager::pin(p, 0);   // SIG1 = SDA
        uint8_t    scl = PortManager::pin(p, 1);   // SIG2 = SCL
        Wire1.begin(sda, scl, CH_I2C1_FREQ_HZ);
        s_begun = true;
    }
    return &Wire1;
}
