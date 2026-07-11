#include "PhoneSensor.h"
#include "../core/CommHub.h"

using namespace ControlHubProtocol;

PhoneSensor::PhoneSensor(uint8_t id) : _id(id) {}

void PhoneSensor::begin()
{
    // BLE transport is already started by ControlHub.begin() - nothing to do here.
}

void PhoneSensor::handleFrame(const Frame &f, CommHub &hub)
{
    (void)hub;
    if (f.moduleType != MOD_PHONE_SENSOR || f.moduleId != _id) return;
    if (f.rtr) return;   // push-only, no snapshot request supported

    switch (f.msgId) {
        case PHONE_MSG_ACCEL:
            if (f.dlc < 6) return;
            _ax = (int16_t)((f.data[0] << 8) | f.data[1]) * 0.001f;
            _ay = (int16_t)((f.data[2] << 8) | f.data[3]) * 0.001f;
            _az = (int16_t)((f.data[4] << 8) | f.data[5]) * 0.001f;
            break;

        case PHONE_MSG_GYRO:
            if (f.dlc < 6) return;
            _gx = (int16_t)((f.data[0] << 8) | f.data[1]) * 0.1f;
            _gy = (int16_t)((f.data[2] << 8) | f.data[3]) * 0.1f;
            _gz = (int16_t)((f.data[4] << 8) | f.data[5]) * 0.1f;
            break;

        case PHONE_MSG_MAG:
            if (f.dlc < 6) return;
            _mx = (int16_t)((f.data[0] << 8) | f.data[1]) * 0.1f;
            _my = (int16_t)((f.data[2] << 8) | f.data[3]) * 0.1f;
            _mz = (int16_t)((f.data[4] << 8) | f.data[5]) * 0.1f;
            break;

        case PHONE_MSG_GPS: {
            if (f.dlc < 8) return;
            int32_t latRaw = ((int32_t)f.data[0] << 24) | ((int32_t)f.data[1] << 16) |
                              ((int32_t)f.data[2] << 8)  |  (int32_t)f.data[3];
            int32_t lonRaw = ((int32_t)f.data[4] << 24) | ((int32_t)f.data[5] << 16) |
                              ((int32_t)f.data[6] << 8)  |  (int32_t)f.data[7];
            _lat = latRaw * 1e-7;
            _lon = lonRaw * 1e-7;
            break;
        }

        case PHONE_MSG_GPS_ALT:
            if (f.dlc < 2) return;
            _alt = (int16_t)((f.data[0] << 8) | f.data[1]) * 0.1f;
            break;

        default:
            return;
    }

    _lastMs = millis();
    _everRx = true;
    if (_cb) _cb(*this);
}
