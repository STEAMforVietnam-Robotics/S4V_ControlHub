#include "ControlHub.h"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

using namespace ControlHubProtocol;

// Global singleton the sketches interact with
_ControlHub ControlHub;

// Global PCA9685 shared by every Motor (declared extern in Motor.h).
Adafruit_PWMServoDriver pca(CH_PCA9685_ADDR);

_ControlHub::_ControlHub() = default;

// Bring up the I2C bus + PCA9685 once. Safe to call multiple times.
static void _initPca()
{
    static bool done = false;
    if (done) return;
    Wire.begin(CH_I2C_SDA, CH_I2C_SCL, CH_I2C_FREQ);
    pca.begin();
    pca.setPWMFreq(CH_PCA9685_FREQ);
    done = true;
}

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
void _ControlHub::begin()
{
    begin(true, true, true, CH_BLE_DEFAULT_NAME);
}

void _ControlHub::begin(bool enUsb, bool enBle, bool enRs485,
                            const char *bleName)
{
    _initPca();
    if (enUsb)   _hub.beginUsb(CH_USB_BAUD);
    if (enBle)   _hub.beginBle(bleName);
    if (enRs485) { _hub.beginRs485(CH_RS485_BAUD); _hub.beginRs485_2(CH_RS485_2_BAUD); }
    _installDefaultHandlers();
}

void _ControlHub::beginUSB  (unsigned long baud) { _initPca(); _hub.beginUsb  (baud); _installDefaultHandlers(); }
void _ControlHub::beginBLE  (const char *n)      { _initPca(); _hub.beginBle  (n);    _installDefaultHandlers(); }
void _ControlHub::beginRS485(unsigned long baud)   { _initPca(); _hub.beginRs485(baud);   _installDefaultHandlers(); }
void _ControlHub::beginRS485_2(unsigned long baud) { _initPca(); _hub.beginRs485_2(baud); _installDefaultHandlers(); }

// ---------------------------------------------------------------------------
//  Update
// ---------------------------------------------------------------------------
void _ControlHub::update()
{
    _hub.update();
}

// ---------------------------------------------------------------------------
//  Default frame handlers - route host frames to the right module
// ---------------------------------------------------------------------------
// Frames from either RS485 bus are *replies* from Smart Modules, not new
// requests to the hub. Anything from RS485_1/RS485_2 that doesn't match an
// on-board module is re-broadcast to the host side (USB / BLE).
static inline bool fromHost(const char *src)
{
    return strcmp(src, "RS485") != 0 && strcmp(src, "RS485_2") != 0;
}

// Route one frame: hand it to the matching on-board module. Host frames that
// match nothing are dropped (sketches address Smart Modules explicitly via
// sendRs485()/sendRs485_2()); smart-module replies are relayed to the host.
void _ControlHub::_route(const Frame &f, const char *src)
{
    for (Module *m = Module::head(); m; m = m->next()) {
        if (m->moduleType() == f.moduleType && m->id() == f.moduleId) {
            m->handleFrame(f, _hub);
            return;
        }
    }
    if (!fromHost(src)) _hub.sendFrame(f);   // relay smart-module reply -> host
}

void _ControlHub::_installDefaultHandlers()
{
    FrameHandler route = [this](const Frame &f, const char *src) { _route(f, src); };

    // Register the same router for every module type the hub knows about.
    _hub.onModule(MOD_MOTOR,      route);
    _hub.onModule(MOD_SERVO,      route);
    _hub.onModule(MOD_ULTRASONIC, route);
    _hub.onModule(MOD_BUMPER,     route);
    _hub.onModule(MOD_TOF,        route);   // Smart ToF modules (proxy to RS485)
    _hub.onModule(MOD_POWER,      route);
    _hub.onModule(MOD_IMU,        route);
    _hub.onModule(MOD_LED,        route);   // WS2812 LED module
    _hub.onModule(MOD_GAMEPAD,    route);   // BLE gamepad
}

// ---------------------------------------------------------------------------
//  On-board WS2812 status LED(s)
// ---------------------------------------------------------------------------
void _ControlHub::_initStatusLed()
{
    if (_statusBegun) return;
    _statusLed.begin();
    _statusLed.setBrightness(_statusBrightness);
    _statusLed.clear();
    _statusLed.show();
    _statusBegun = true;
}

void _ControlHub::statusColor(uint8_t r, uint8_t g, uint8_t b)
{
    _initStatusLed();
    _statusLed.fill(_statusLed.Color(r, g, b));
    _statusLed.show();
}

void _ControlHub::statusPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
    _initStatusLed();
    if (i >= (uint16_t)CH_LED_STATUS_NUM) return;
    _statusLed.setPixelColor(i, _statusLed.Color(r, g, b));
    _statusLed.show();
}

void _ControlHub::statusBrightness(uint8_t b)
{
    _statusBrightness = b;
    _initStatusLed();
    _statusLed.setBrightness(b);
    _statusLed.show();
}

void _ControlHub::statusBlink(uint8_t r, uint8_t g, uint8_t b)
{
    _initStatusLed();
    _statusBlinkOn = !_statusBlinkOn;
    if (_statusBlinkOn) _statusLed.fill(_statusLed.Color(r, g, b));
    else                _statusLed.clear();
    _statusLed.show();
}

void _ControlHub::statusOff()
{
    _initStatusLed();
    _statusLed.clear();
    _statusLed.show();
}
