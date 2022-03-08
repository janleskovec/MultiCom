#ifndef multicom_ble_h
#define multicom_ble_h

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "MultiCom.h"

class MultiComBle;

class _ble_server_callbacks: public BLEServerCallbacks{
  public:
    _ble_server_callbacks(MultiComBle *channel);
    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);
  private:
    MultiComBle *_channel;
};
class _ble_callbacks: public BLECharacteristicCallbacks{
  public:
    _ble_callbacks(MultiComBle *channel);
    void onWrite(BLECharacteristic *pCharacteristic);
  private:
    MultiComBle *_channel;
};


class MultiComBle : public MultiComChannel {

  public:
    MultiComBle(const char *bt_name);
    bool start();
    void stop();

    bool _deviceConnected = false;
    void _send(void *data, u16_t len);

  private:
    const char* _bt_name;
    bool _connected;

    BLEServer *_pServer = NULL;
    BLECharacteristic * _pTxCharacteristic;
};


#endif
