#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <functional>

#include "MultiCom.h"

MultiComMsg::MultiComMsg(void *n_data, u16_t n_len, MultiComReplyFn *n_reply){
  data = n_data;
  len = n_len;
  reply = n_reply;
}

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
  channelUdp = udp;
  // TODO: fix
  channelUdp->_callback = std::bind1st(&_onNewMsg, this);
  //channelBle = ble;
  //channelBle->_callback = std::bind1st(_onNewMsg, this);
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

void MultiCom::_onNewMsg(MultiComMsg *msg){
  Serial.print("_onNewMsg: ");
  for (int i = 0; i < msg->len; i++) {
    Serial.printf("%4d ", ((char*) msg->data)[i]);
  }
  Serial.print("\n           ");
  for (int i = 0; i < msg->len; i++) {
    Serial.printf("   %c ", ((char*) msg->data)[i]);
  }
  Serial.println();
}

