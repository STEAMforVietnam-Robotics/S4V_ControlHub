#pragma once

#include "IDevice.h"

class IDigitalIO : public IDevice {
public:
    virtual ~IDigitalIO() = default;
    // True when the input is "active" (e.g. button pressed, switch closed).
    virtual bool value() = 0;
};
