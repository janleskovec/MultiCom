#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "MultiCom.h"
#include "MultiComBle.h"

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

_ble_server_callbacks::_ble_server_callbacks(MultiComBle *channel) {
  _channel = channel;
}
void _ble_server_callbacks::onConnect(BLEServer* pServer) {
  _channel->_deviceConnected = true;
}
void _ble_server_callbacks::onDisconnect(BLEServer* pServer) {
  _channel->_deviceConnected = false;
  pServer->startAdvertising();
}

_ble_callbacks::_ble_callbacks(MultiComBle *channel) {
  _channel = channel;
}
void _ble_callbacks::onWrite(BLECharacteristic *pCharacteristic) {
  std::string rxValue = pCharacteristic->getValue();

  if (rxValue.length() > 0) {
    for (int i = 0; i < rxValue.length(); i++) {
      Serial.print(rxValue[i]);
    }

    if (_channel->_callback != NULL)
      _channel->_callback(
        (void*)rxValue.c_str(),
        rxValue.length(),
        // reply lambda
        [this](void *data, u16_t len){ this->_channel->_send(data, len); }
      );
  }
}


MultiComBle::MultiComBle(const char *bt_name) {
  _bt_name = bt_name;
}

bool MultiComBle::start() {
  bool success = true;

  Serial.println("start ble server");

  BLEDevice::init(_bt_name);
  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(new _ble_server_callbacks(this));
  BLEService *pService = _pServer->createService(SERVICE_UUID);
  _pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);      
  _pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);
  pRxCharacteristic->setCallbacks(new _ble_callbacks(this));
  pService->start();
  _pServer->getAdvertising()->start();

  isRunning = success;

  return success;
}

void MultiComBle::stop() {
  isRunning = false;
  Serial.println("stop ble server");

  // TODO: implement
}


// TODO: check if same device connected
void MultiComBle::_send(void *data, u16_t len) {
  _pTxCharacteristic->setValue((u8_t*)data, len);
  _pTxCharacteristic->notify();
}
