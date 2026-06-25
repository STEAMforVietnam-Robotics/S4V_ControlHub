#include "SysAPI.h"
#include "../devices/Motor.h"
#include <Arduino.h>

SysAPI Sys;

void SysAPI::stop()                       { motor.stopAll(STOP_COAST); }
void SysAPI::wait(float s)                { if (s > 0) delay((uint32_t)(s * 1000.0f)); }
void SysAPI::wait_sec(float s)            { wait(s); }
void SysAPI::wait_millisec(uint32_t ms)   { delay(ms); }
float    SysAPI::uptime_sec() const       { return millis() * 0.001f; }
uint32_t SysAPI::uptime_ms()  const       { return millis(); }
