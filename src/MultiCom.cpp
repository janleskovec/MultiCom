#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <functional>

#include "MultiCom.h"


bool MultiComChannel::start() {
  Serial.println("start generic server");
  isRunning = true;
  return true;
}

void MultiComChannel::stop() {
  Serial.println("stop generic server");
  isRunning = false;
}


MultiCom::MultiCom(MultiComChannel *udp) {
  using namespace std::placeholders;

  MultiComNewDataFn _callback = std::bind(&MultiCom::_onNewMsg, this, _1, _2, _3);

  channelUdp = udp;
  channelUdp->_callback = _callback;
  //channelBle = ble;
  //channelBle->_callback = _callback;
}

bool MultiCom::startAll() {
  bool success = true;

  if (channelUdp != NULL) success &= channelUdp->start();
  // if (channelBle != NULL) success &= channelBle->start();

  if (!success) {
    channelUdp->stop();
    //channelBle->stop();
  }

  return success;
}

void MultiCom::_onNewMsg(void *data, u16_t len, MultiComReplyFn reply){
  Serial.print("_onNewMsg: ");
  for (int i = 0; i < len; i++) {
    Serial.printf("%4d ", ((char*) data)[i]);
  }
  Serial.print("\n           ");
  for (int i = 0; i < len; i++) {
    Serial.printf("   %c ", ((char*) data)[i]);
  }
  Serial.println();
}

