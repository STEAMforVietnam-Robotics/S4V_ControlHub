#include "iface/IDistanceSensor.h"
#include "DeviceRegistry.h"

float DistanceHandle::distance_cm() const {
    IDistanceSensor* d = DeviceRegistry::asDistance(_slot);
    return d ? d->distance_cm() : -1.0f;
}
