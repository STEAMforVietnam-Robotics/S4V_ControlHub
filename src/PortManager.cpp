#include "PortManager.h"
#include "ch_config.h"
#include <Arduino.h>

// ----------------------------------------------------------------------------
//  Static capability + pin table (one row per port).
//  Reflects the hardware notes in the README and Phase-plan section 4.
// ----------------------------------------------------------------------------
namespace {
    struct PortRow {
        uint8_t  sig1;
        uint8_t  sig2;       // 0xFF if port has only one signal line
        uint16_t caps;
        const char* name;
    };

    // Order matches enum SensorPort (PORT_MF4P_1..PORT_MF3P_3).
    constexpr PortRow kPorts[] = {
        // PORT_MF4P_1: GPIO16, GPIO17 — two GPIOs, PWM, I2C, RMT (no ADC on these pins on S3)
        { CH_MF4P_1_SIG1, CH_MF4P_1_SIG2,
          CAP_GPIO_DIGITAL | CAP_GPIO_TWO | CAP_PWM | CAP_I2C | CAP_RMT,
          "MF4P_1" },
        // PORT_MF4P_2: GPIO18, GPIO8 — same caps as MF4P_1
        //   (GPIO19 is reserved for ESP32-S3 native USB D-)
        { CH_MF4P_2_SIG1, CH_MF4P_2_SIG2,
          CAP_GPIO_DIGITAL | CAP_GPIO_TWO | CAP_PWM | CAP_I2C | CAP_RMT,
          "MF4P_2" },
        // PORT_MF3P_1: GPIO9 — single GPIO, ADC1, PWM, RMT
        { CH_MF3P_1_SIG, 0xFF,
          CAP_GPIO_DIGITAL | CAP_ADC | CAP_PWM | CAP_RMT,
          "MF3P_1" },
        // PORT_MF3P_2: GPIO10 — same as MF3P_1
        { CH_MF3P_2_SIG, 0xFF,
          CAP_GPIO_DIGITAL | CAP_ADC | CAP_PWM | CAP_RMT,
          "MF3P_2" },
        // PORT_MF3P_3: GPIO11 — ADC2 (conflicts with WiFi when active), other caps same
        { CH_MF3P_3_SIG, 0xFF,
          CAP_GPIO_DIGITAL | CAP_ADC | CAP_PWM | CAP_RMT,
          "MF3P_3" },
    };
    constexpr uint8_t PORT_COUNT = sizeof(kPorts) / sizeof(kPorts[0]);

    DeviceKind s_owner[PORT_COUNT] = { DEV_NONE };

    // Shared Wire1 bus state. At most one MF4P port hosts it.
    bool       s_i2c_bound = false;
    SensorPort s_i2c_port  = PORT_MF4P_1;          // valid only when s_i2c_bound
    uint8_t    s_i2c_addr[CH_MAX_I2C_DEVICES];
    uint8_t    s_i2c_count = 0;

    int portIndex(SensorPort port) {
        int i = (int)port - (int)PORT_MF4P_1;     // PORT_MF4P_1 == 0
        return (i >= 0 && i < PORT_COUNT) ? i : -1;
    }
}

// ----------------------------------------------------------------------------
bool PortManager::claim(SensorPort port, DeviceKind kind, uint16_t required_caps) {
    int idx = portIndex(port);
    if (idx < 0) {
        Serial.printf("[ControlHub] claim FAILED: invalid port id\n");
        return false;
    }
    const PortRow& row = kPorts[idx];

    // Capability check.
    if ((row.caps & required_caps) != required_caps) {
        Serial.printf("[ControlHub] %s does not support %s (caps 0x%04X needed, has 0x%04X)\n",
                      row.name, deviceName(kind), required_caps, row.caps);
        return false;
    }
    // Reject if the port is currently hosting the shared Wire1 bus.
    if (s_i2c_bound && s_i2c_port == port) {
        Serial.printf("[ControlHub] %s is the I2C1 bus, %s rejected\n",
                      row.name, deviceName(kind));
        return false;
    }
    // Ownership check.
    if (s_owner[idx] != DEV_NONE) {
        Serial.printf("[ControlHub] %s already owned by %s, %s rejected\n",
                      row.name, deviceName(s_owner[idx]), deviceName(kind));
        return false;
    }
    s_owner[idx] = kind;
    return true;
}

void PortManager::release(SensorPort port) {
    int idx = portIndex(port);
    if (idx >= 0) s_owner[idx] = DEV_NONE;
}

// ----------------------------------------------------------------------------
bool PortManager::claimI2C(SensorPort port, DeviceKind kind, uint8_t addr) {
    int idx = portIndex(port);
    if (idx < 0) {
        Serial.printf("[ControlHub] claimI2C FAILED: invalid port id\n");
        return false;
    }
    const PortRow& row = kPorts[idx];

    // Must be an I2C-capable port (only the 4P ports carry CAP_I2C).
    if (!(row.caps & CAP_I2C)) {
        Serial.printf("[ControlHub] %s does not support I2C, %s rejected\n",
                      row.name, deviceName(kind));
        return false;
    }
    // Port must not already belong to an exclusive (non-I2C) device.
    if (s_owner[idx] != DEV_NONE) {
        Serial.printf("[ControlHub] %s already owned by %s, I2C %s rejected\n",
                      row.name, deviceName(s_owner[idx]), deviceName(kind));
        return false;
    }
    // Only one MF4P port may host Wire1.
    if (s_i2c_bound && s_i2c_port != port) {
        Serial.printf("[ControlHub] I2C1 already on %s; %s cannot use %s\n",
                      portName(s_i2c_port), deviceName(kind), row.name);
        return false;
    }
    // No two devices at the same address on one bus.
    for (uint8_t i = 0; i < s_i2c_count; ++i) {
        if (s_i2c_addr[i] == addr) {
            Serial.printf("[ControlHub] I2C addr 0x%02X already in use on %s\n",
                          addr, row.name);
            return false;
        }
    }
    if (s_i2c_count >= CH_MAX_I2C_DEVICES) {
        Serial.printf("[ControlHub] I2C device limit reached — increase CH_MAX_I2C_DEVICES\n");
        return false;
    }
    s_i2c_bound = true;
    s_i2c_port  = port;
    s_i2c_addr[s_i2c_count++] = addr;
    return true;
}

void PortManager::releaseI2C(SensorPort port, uint8_t addr) {
    (void)port;
    for (uint8_t i = 0; i < s_i2c_count; ++i) {
        if (s_i2c_addr[i] == addr) {
            s_i2c_addr[i] = s_i2c_addr[--s_i2c_count];   // compact
            break;
        }
    }
    if (s_i2c_count == 0) s_i2c_bound = false;
}

bool       PortManager::i2cBound() { return s_i2c_bound; }
SensorPort PortManager::i2cPort()  { return s_i2c_port; }

uint16_t PortManager::capabilities(SensorPort port) {
    int idx = portIndex(port);
    return (idx >= 0) ? kPorts[idx].caps : 0;
}

bool PortManager::supports(SensorPort port, uint16_t required_caps) {
    return (capabilities(port) & required_caps) == required_caps;
}

uint8_t PortManager::pin(SensorPort port, uint8_t which) {
    int idx = portIndex(port);
    if (idx < 0) return 0xFF;
    return (which == 0) ? kPorts[idx].sig1 : kPorts[idx].sig2;
}

DeviceKind PortManager::owner(SensorPort port) {
    int idx = portIndex(port);
    return (idx >= 0) ? s_owner[idx] : DEV_NONE;
}

const char* PortManager::portName(SensorPort port) {
    int idx = portIndex(port);
    return (idx >= 0) ? kPorts[idx].name : "?";
}

const char* PortManager::deviceName(DeviceKind k) {
    switch (k) {
        case DEV_NONE:        return "(free)";
        case DEV_ULTRASONIC:  return "ULTRASONIC";
        case DEV_TOF:         return "TOF";
        case DEV_SHARP_IR:    return "SHARP_IR";
        case DEV_CONTACT:     return "CONTACT";
        case DEV_POT:         return "POT";
        case DEV_NEOPIXEL:    return "NEOPIXEL";
        case DEV_BUZZER:      return "BUZZER";
        case DEV_DIGITAL_OUT: return "DIGITAL_OUT";
    }
    return "?";
}
