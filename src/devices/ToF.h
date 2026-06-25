// ============================================================================
//  devices/ToF.h — VL53L0X Time-of-Flight distance sensor on a 4P port (I²C).
//
//  Runs on the secondary bus Wire1 (I2C1), NOT the PCA9685 bus. Cable the
//  VL53L0X to a 4P port: SIG1 = SDA, SIG2 = SCL. The first I²C device created
//  binds Wire1 to its port; further I²C devices must share that same port and
//  use distinct addresses. Only one of MF4P_1 / MF4P_2 can host the bus.
//
//  VL53L0X default address is 0x29. For two VL53L0X at once you must re-address
//  one via its XSHUT pin (extra GPIO required).
// ============================================================================
#pragma once

#include "../ch_types.h"
#include "../iface/IDistanceSensor.h"

class Adafruit_VL53L0X;   // forward decl

class ToF : public IDistanceSensor {
public:
    explicit ToF(SensorPort port, uint8_t i2c_addr = 0x29);
    ~ToF() override;

    bool hwInit() override;
    float distance_cm() override;
    float distance_mm();

    bool ready() const { return _ok; }

private:
    SensorPort         _port;
    uint8_t            _addr;
    bool               _ok;
    Adafruit_VL53L0X*  _sensor;
};

class ToFHandle {
public:
    ToFHandle() : _slot(INVALID_SLOT) {}
    explicit ToFHandle(uint8_t slot) : _slot(slot) {}
    float distance_cm() const;
    float distance_mm() const;
    bool  valid() const { return _slot != INVALID_SLOT; }
    operator DistanceHandle() const { return DistanceHandle(_slot); }
private:
    uint8_t _slot;
};
