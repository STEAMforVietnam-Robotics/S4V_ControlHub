// ============================================================================
//  SysAPI.h — Level-2 system.* equivalent (singleton `Sys`)
//
//  Uses `Sys` (capital S) to avoid shadowing <cstdlib>::system().
// ============================================================================
#pragma once

#include <stdint.h>

class SysAPI {
public:
    void stop();
    void wait(float seconds);
    void wait_sec(float seconds);
    void wait_millisec(uint32_t ms);
    float    uptime_sec() const;
    uint32_t uptime_ms() const;
};

extern SysAPI Sys;
