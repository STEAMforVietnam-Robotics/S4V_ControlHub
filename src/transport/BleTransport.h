/*  BleTransport.h
 *
 *  BLE UART bridge (Nordic UART Service - NUS).
 *  Host writes frames to RX characteristic; hub streams frames via TX
 *  characteristic notifications. Works with nRF Connect and most maker apps.
 */

#ifndef CONTROLHUB_BLE_TRANSPORT_H
#define CONTROLHUB_BLE_TRANSPORT_H

#include "Transport.h"
#include "../ControlHubConfig.h"

// Only compile BLE code on ESP32 family; guard so the rest of the library
// still builds if BLE is stripped out for size reasons.
#if defined(ESP32) && !defined(CH_NO_BLE)
#define CH_HAS_BLE  1
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#endif

namespace ControlHubProtocol {

class BleTransport : public Transport {
public:
    BleTransport();

    /** Start advertising as @p name. Silently no-op if BLE compiled out. */
    void begin(const char *name = CH_BLE_DEFAULT_NAME);
    void end();

    int    available() override;
    int    read()      override;
    size_t write(const uint8_t *data, size_t len) override;
    bool   connected() override;
    const char *name() const override { return "BLE"; }

#ifdef CH_HAS_BLE
    // Callbacks used internally by BLE stack (must be public for delegates)
    void _onClientWrite(const uint8_t *data, size_t len);
    void _onConnect();
    void _onDisconnect();
#endif

private:
#ifdef CH_HAS_BLE
    static constexpr size_t RX_BUF_SIZE = 512;

    BLEServer          *_server  = nullptr;
    BLEService         *_service = nullptr;
    BLECharacteristic  *_txChar  = nullptr;
    BLECharacteristic  *_rxChar  = nullptr;

    // Simple ring buffer for bytes received from host
    volatile uint16_t   _head = 0, _tail = 0;
    uint8_t             _rx[RX_BUF_SIZE] = {0};

    volatile bool       _connected = false;
    bool                _started   = false;
#endif
};

} // namespace
#endif
