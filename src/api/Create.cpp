#include "Create.h"
#include "../ControlHub.h"
#include "../DeviceRegistry.h"
#include "../PortManager.h"

// Build helper: if the device claimed its port, register and return handle.
// Otherwise delete the half-built device and return an invalid handle.
//
// The device's hardware is initialised either now (if create() runs after
// ControlHub.begin()) or later from ControlHub.begin() (the global-scope case,
// where touching peripherals in a constructor before initArduino() would crash).
template <typename Dev, typename Handle>
static Handle _build(DeviceKind kind, SensorPort port, Dev* dev) {
    if (PortManager::owner(port) != kind) {
        delete dev;
        return Handle();
    }
    uint8_t slot = DeviceRegistry::allocate(kind, port, dev);
    if (slot == INVALID_SLOT) {
        PortManager::release(port);   // dev owned the port; give it back
        delete dev;
        return Handle();
    }
    if (ControlHub.isReady()) DeviceRegistry::initSlot(slot);
    return Handle(slot);
}

// Build helper for shared-bus I2C devices. Unlike _build (which re-queries
// exclusive port ownership), success is read from the device's own claim flag,
// so several I2C devices can share one port. On failure the device's destructor
// releases its bus address.
template <typename Dev, typename Handle>
static Handle _buildI2C(DeviceKind kind, SensorPort port, Dev* dev) {
    if (!dev->portClaimed()) {
        delete dev;
        return Handle();
    }
    uint8_t slot = DeviceRegistry::allocate(kind, port, dev);
    if (slot == INVALID_SLOT) {
        delete dev;
        return Handle();
    }
    if (ControlHub.isReady()) DeviceRegistry::initSlot(slot);
    return Handle(slot);
}

UltrasonicHandle create(ch_tag::Ultrasonic_t, SensorPort port) {
    return _build<Ultrasonic, UltrasonicHandle>(
        DEV_ULTRASONIC, port, new Ultrasonic(port));
}

ToFHandle create(ch_tag::ToF_t, SensorPort port, uint8_t i2c_addr) {
    return _buildI2C<ToF, ToFHandle>(
        DEV_TOF, port, new ToF(port, i2c_addr));
}

SharpIRHandle create(ch_tag::SharpIR_t, SensorPort port) {
    return _build<SharpIR, SharpIRHandle>(
        DEV_SHARP_IR, port, new SharpIR(port));
}

ContactHandle create(ch_tag::Contact_t, SensorPort port, bool active_high) {
    return _build<Contact, ContactHandle>(
        DEV_CONTACT, port, new Contact(port, active_high));
}

PotHandle create(ch_tag::Pot_t, SensorPort port) {
    return _build<Pot, PotHandle>(DEV_POT, port, new Pot(port));
}

DigitalOutHandle create(ch_tag::DigitalOut_t, SensorPort port, bool initial_state) {
    return _build<DigitalOut, DigitalOutHandle>(
        DEV_DIGITAL_OUT, port, new DigitalOut(port, initial_state));
}

NeoPixelHandle create(ch_tag::NeoPixel_t, SensorPort port, uint16_t num_pixels) {
    return _build<NeoPixel, NeoPixelHandle>(
        DEV_NEOPIXEL, port, new NeoPixel(port, num_pixels));
}

BuzzerHandle create(ch_tag::Buzzer_t, SensorPort port) {
    return _build<Buzzer, BuzzerHandle>(DEV_BUZZER, port, new Buzzer(port));
}
