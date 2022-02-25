#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <functional>

#include "MultiCom.h"

#include "MultiComPacket.h"


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

void MultiCom::_endpointRouter(MultiComPacket packet, MultiComReplyFn reply){

  switch (packet.type)
  {
  case MultiComPacket::packet_type::get:
    reply((void*)"get", strlen("get"));
    break;
  
  case MultiComPacket::packet_type::send:
    reply((void*)"send", strlen("send"));
    break;
  
  case MultiComPacket::packet_type::post:
    reply((void*)"post", strlen("post"));
    break;
  
  }

  // TODO: for testing only, remove!
  if (tmp_callback != NULL) tmp_callback(packet, reply);
}

void MultiCom::_onNewMsg(void *data, u16_t len, MultiComReplyFn reply){
  MultiComPacket packet = MultiComPacket(data, len);

  switch (packet.type)
  {
  case MultiComPacket::packet_type::discovery:
    reply((void*)"discovery", strlen("discovery"));
    break;
  
  case MultiComPacket::packet_type::get:
  case MultiComPacket::packet_type::send:
  case MultiComPacket::packet_type::post:
    reply((void*)"router", strlen("router"));
    _endpointRouter(packet, reply);
    break;
  
  // ack should not be received
  case MultiComPacket::packet_type::ack:
    reply((void*)"ack", strlen("ack"));
    break;
  
  default:
    Serial.printf("unknown packet type: %hhu\n", packet.type);
    reply((void*)"unknown", strlen("unknown"));
    break;
  }
}

