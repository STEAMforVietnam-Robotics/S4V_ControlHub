#include "Ultrasonic.h"
#include "../PortManager.h"
#include "../DeviceRegistry.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
//  Concrete driver
// ---------------------------------------------------------------------------
Ultrasonic::Ultrasonic(SensorPort port)
    : _port(port), _trig(0xFF), _echo(0xFF), _ok(false),
      _timeout_us(30000UL)
{
    // Software-only at construction (may run before initArduino()):
    // HC-SR04 needs two GPIO lines → CAP_GPIO_TWO is mandatory.
    PortManager::claim(port, DEV_ULTRASONIC, CAP_GPIO_DIGITAL | CAP_GPIO_TWO);
}

bool Ultrasonic::hwInit() {
    _trig = PortManager::pin(_port, 0);
    _echo = PortManager::pin(_port, 1);
    if (_trig == 0xFF || _echo == 0xFF) return false;
    pinMode(_trig, OUTPUT);
    pinMode(_echo, INPUT);
    digitalWrite(_trig, LOW);
    _ok = true;
    return true;
}

Ultrasonic::~Ultrasonic() {
    if (_ok) PortManager::release(_port);
}

float Ultrasonic::distance_cm() {
    if (!_ok) return -1.0f;
    digitalWrite(_trig, LOW);
    delayMicroseconds(2);
    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);
    unsigned long us = pulseIn(_echo, HIGH, _timeout_us);
    if (us == 0) return -1.0f;
    return (float)us / 58.0f;
}

// ---------------------------------------------------------------------------
//  Handle proxy methods
// ---------------------------------------------------------------------------
float UltrasonicHandle::distance_cm() const {
    IDistanceSensor* d = DeviceRegistry::asDistance(_slot);
    return d ? d->distance_cm() : -1.0f;
}
