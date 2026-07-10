#include "BleTransport.h"

namespace ControlHubProtocol {

// ---------------------------------------------------------------------------
//  Stub when BLE is compiled out
// ---------------------------------------------------------------------------
#ifndef CH_HAS_BLE

BleTransport::BleTransport() {}
void   BleTransport::begin(const char *) {}
void   BleTransport::end() {}
int    BleTransport::available()          { return 0; }
int    BleTransport::read()               { return -1; }
size_t BleTransport::write(const uint8_t *, size_t) { return 0; }
bool   BleTransport::connected()          { return false; }

#else

// ---------------------------------------------------------------------------
//  Real ESP32 BLE implementation (Nordic UART Service)
// ---------------------------------------------------------------------------

namespace {

class ServerCb : public BLEServerCallbacks {
public:
    explicit ServerCb(BleTransport *t) : _t(t) {}
    void onConnect   (BLEServer *) override { _t->_onConnect();    }
    void onDisconnect(BLEServer *s) override {
        _t->_onDisconnect();
        // Immediately re-advertise so a fresh client can connect
        s->getAdvertising()->start();
    }
private:
    BleTransport *_t;
};

class RxCb : public BLECharacteristicCallbacks {
public:
    explicit RxCb(BleTransport *t) : _t(t) {}
    void onWrite(BLECharacteristic *c) override {
        String v = c->getValue();
        _t->_onClientWrite(reinterpret_cast<const uint8_t*>(v.c_str()), v.length());
    }
private:
    BleTransport *_t;
};

} // anon namespace

BleTransport::BleTransport() {}

void BleTransport::begin(const char *name)
{
    if (_started) return;

    BLEDevice::init(name);
    BLEDevice::setMTU(247);            // request a larger MTU when peer allows

    _server = BLEDevice::createServer();
    _server->setCallbacks(new ServerCb(this));

    _service = _server->createService(CH_BLE_NUS_SERVICE_UUID);

    _txChar = _service->createCharacteristic(
                 CH_BLE_NUS_TX_CHAR_UUID,
                 BLECharacteristic::PROPERTY_NOTIFY);
    _txChar->addDescriptor(new BLE2902());

    _rxChar = _service->createCharacteristic(
                 CH_BLE_NUS_RX_CHAR_UUID,
                 BLECharacteristic::PROPERTY_WRITE |
                 BLECharacteristic::PROPERTY_WRITE_NR);
    _rxChar->setCallbacks(new RxCb(this));

    _service->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(CH_BLE_NUS_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    _started = true;
}

void BleTransport::end()
{
    if (!_started) return;
    BLEDevice::deinit(true);
    _started   = false;
    _connected = false;
}

void BleTransport::_onConnect()    { _connected = true;  }
void BleTransport::_onDisconnect() { _connected = false; }

void BleTransport::_onClientWrite(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        uint16_t next = (_head + 1) % RX_BUF_SIZE;
        if (next == _tail) return;     // drop on overflow
        _rx[_head] = data[i];
        _head = next;
    }
}

int BleTransport::available()
{
    if (!_started) return 0;
    return (_head - _tail + RX_BUF_SIZE) % RX_BUF_SIZE;
}

int BleTransport::read()
{
    if (!_started || _head == _tail) return -1;
    uint8_t b = _rx[_tail];
    _tail = (_tail + 1) % RX_BUF_SIZE;
    return b;
}

size_t BleTransport::write(const uint8_t *data, size_t len)
{
    if (!_started || !_connected || !_txChar) return 0;

    // Split into MTU-sized notifications (safe default = 20 for 23-byte MTU)
    constexpr size_t CHUNK = 180;      // works with the 247-byte MTU we ask for
    size_t sent = 0;
    while (sent < len) {
        size_t n = min(CHUNK, len - sent);
        _txChar->setValue(const_cast<uint8_t*>(data + sent), n);
        _txChar->notify();
        sent += n;
        delay(1);                       // give BLE stack a breather
    }
    return sent;
}

bool BleTransport::connected() { return _started && _connected; }

#endif  // CH_HAS_BLE

} // namespace
