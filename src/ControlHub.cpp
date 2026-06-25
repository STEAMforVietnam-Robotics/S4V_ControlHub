#include "ControlHub.h"
#include "DeviceRegistry.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


_ControlHub ControlHub;

static Adafruit_PWMServoDriver s_pwm(CH_PCA9685_ADDR);

void _ControlHub::begin() {
    if (_ready) return;

    Wire.begin(CH_I2C_SDA, CH_I2C_SCL, CH_I2C_FREQ_HZ);

    s_pwm.begin();
    s_pwm.setOscillatorFrequency(27000000);
    s_pwm.setPWMFreq(CH_PCA9685_PWM_FREQ_HZ);
    delay(10);
    this->ledBoard = new Adafruit_NeoPixel(LED_BOARD_NUM, CH_LED_PIN, NEO_GRB + NEO_KHZ800);    
    this->ledBoard->begin();

    // Core hardware (always present). Pluggable devices come from create().
    motor._init(&s_pwm);
    servo._init();
    robot._init();

    // Initialise every device registered via create() at global scope. Their
    // constructors only claimed ports (safe before initArduino()); the actual
    // peripheral setup (pinMode/RMT/LEDC/I²C) happens here, now that Wire and
    // the Arduino HAL are up.
    DeviceRegistry::initAll();
    _ready = true;
}
