/*  PhoneSensor.h
 *
 *  Accel / Gyro / Mag / GPS pushed from a phone app over the on-board BLE
 *  Nordic UART Service - push-only, no request/reply (mirrors Gamepad).
 *
 *      PhoneSensor phone;
 *      void setup(){ ControlHub.begin(); phone.begin(); }
 *      void loop(){
 *          ControlHub.update();
 *          if (phone.connected()) Serial.println(phone.accelX());
 *      }
 *
 *  Wire format (MOD_PHONE_SENSOR, one msgId per sensor - see PhoneMsgId):
 *      PHONE_MSG_ACCEL   data[0..5] = accelX/Y/Z, int16 big-endian, mg (1000=1g)
 *      PHONE_MSG_GYRO    data[0..5] = gyroX/Y/Z,  int16 big-endian, 0.1 deg/s
 *      PHONE_MSG_MAG     data[0..5] = magX/Y/Z,   int16 big-endian, 0.1 uT
 *      PHONE_MSG_GPS     data[0..3] = lat, data[4..7] = lon, int32 big-endian, deg*1e7
 *      PHONE_MSG_GPS_ALT data[0..1] = altitude, int16 big-endian, decimetres
 */

#ifndef CONTROLHUB_PHONESENSOR_H
#define CONTROLHUB_PHONESENSOR_H

#include <Arduino.h>
#include "../ControlHubConfig.h"
#include "../protocol/Frame.h"
#include "Module.h"

namespace ControlHubProtocol { class CommHub; }

class PhoneSensor : public Module {
public:
    /** @param id wire ID (1 for the first / only phone). */
    explicit PhoneSensor(uint8_t id = 1);

    void begin() override;

    // ---- Accelerometer (g), Gyroscope (deg/s), Magnetometer (uT) ---------
    float accelX() const { return _ax; }
    float accelY() const { return _ay; }
    float accelZ() const { return _az; }
    float gyroX()  const { return _gx; }
    float gyroY()  const { return _gy; }
    float gyroZ()  const { return _gz; }
    float magX()   const { return _mx; }
    float magY()   const { return _my; }
    float magZ()   const { return _mz; }

    // ---- GPS ---------------------------------------------------------------
    double latitude()  const { return _lat; }
    double longitude() const { return _lon; }
    float  altitude()  const { return _alt; }   // metres

    /** True while frames keep arriving (auto-clears after the timeout). */
    bool connected() const {
        return _everRx && (millis() - _lastMs < CH_PHONE_SENSOR_TIMEOUT_MS);
    }

    /** Called once per received frame (any sensor type). */
    void onUpdate(void (*cb)(PhoneSensor &)) { _cb = cb; }

    // ---- Module interface ------------------------------------------------
    uint8_t moduleType() const override { return ControlHubProtocol::MOD_PHONE_SENSOR; }
    uint8_t id()         const override { return _id; }

    void handleFrame(const ControlHubProtocol::Frame &f,
                     ControlHubProtocol::CommHub &hub) override;

private:
    uint8_t  _id;

    float _ax = 0, _ay = 0, _az = 0;
    float _gx = 0, _gy = 0, _gz = 0;
    float _mx = 0, _my = 0, _mz = 0;
    double _lat = 0, _lon = 0;
    float  _alt = 0;

    uint32_t _lastMs = 0;
    bool     _everRx = false;
    void (*_cb)(PhoneSensor &) = nullptr;
};

#endif
