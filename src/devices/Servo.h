// ============================================================================
//  devices/Servo.h — Servo manager for 4 RC servo ports (core hardware)
// ============================================================================
#pragma once

#include "../ch_types.h"

class Servo;  // ESP32Servo forward decl

class ServoManager {
public:
    ServoManager();
    void _init();
    void  write(ServoId id, float angle_deg);
    void  write(ServoId id, float angle_deg, float speed_deg_per_s);
    float read(ServoId id) const;
    void  detach(ServoId id);
private:
    Servo* _servo[SERVO_COUNT_];
    float  _last_deg[SERVO_COUNT_];
    static int _idx(ServoId id) { return (int)id - 1; }
};

extern ServoManager servo;
