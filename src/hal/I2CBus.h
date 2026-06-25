// ============================================================================
//  hal/I2CBus.h — Secondary I²C bus (Wire1) for multi-function 4P ports
//
//  PCA9685 lives on the primary Wire (I2C0). Any I²C sensor cabled to a 4P
//  port talks over Wire1 instead. The bus is brought up lazily on whichever
//  MF4P port the first I²C device claimed via PortManager::claimI2C().
// ============================================================================
#pragma once

class TwoWire;

namespace ch_i2c {
    // Return the Wire1 bus, starting it on the bound MF4P port the first time
    // (SIG1 = SDA, SIG2 = SCL). Returns nullptr if no I²C port is bound yet.
    TwoWire* bus1();
}
