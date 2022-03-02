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

  // init sessions list
  //_session_list = {};
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

void MultiCom::_endpointRouter(MultiComPacket packet, MultiComReplyFn reply) {

  MultiCom::session *session = _getSession(packet.session_id);

  Serial.printf("packet_nonce=%u, session_nonce=%u\n", packet.nonce, session->nonce);

  switch (packet.type)
  {
  case MultiComPacket::packet_type::get:
    reply((void*)"get", strlen("get"));
    // reset nonce, just in case as "packet_type::get" should ignore it
    packet.nonce = 0;
    break;
  
  case MultiComPacket::packet_type::send:
    reply((void*)"send", strlen("send"));
    if (packet.nonce > session->nonce) {
      // TODO: for testing only, remove!
      if (tmp_callback != NULL) tmp_callback(packet, reply);
    }

    break;
  
  case MultiComPacket::packet_type::post:
    reply((void*)"post", strlen("post"));
    if (packet.nonce > session->nonce) {
      // TODO: for testing only, remove!
      if (tmp_callback != NULL) tmp_callback(packet, reply);
    }
    
    //TODO: test
    // send ack
    MultiComPacket ackPacket = MultiComPacket::genAckPacket(packet.session_id, packet.nonce);
    reply(ackPacket._raw_data, ackPacket._raw_len);
    free(ackPacket._raw_data); // free buff

    break;
  }

  // increase nonce
  if (packet.nonce > session->nonce) session->nonce = packet.nonce;
}

void MultiCom::_onNewMsg(void *data, u16_t len, MultiComReplyFn reply){
  MultiComPacket packet = MultiComPacket(data, len);

  switch (packet.type)
  {
  case MultiComPacket::packet_type::discovery:
    // generate response
    {MultiComPacket rply_packet = MultiComPacket::genDiscoveryReply(_fw_id, _dev_id, _api_ver);
    reply(rply_packet._raw_data, rply_packet._raw_len);
    free(//TODO)}
    break;
  
  case MultiComPacket::packet_type::ping:
    reply(data, len); // echo
    break;
  
  case MultiComPacket::packet_type::get:
  case MultiComPacket::packet_type::send:
  case MultiComPacket::packet_type::post:
    reply((void*)"router", strlen("router"));
    _endpointRouter(packet, reply);
    break;
  
  // ack should not be received
  case MultiComPacket::packet_type::ack:
  case MultiComPacket::packet_type::get_reply:
    break;
  
  default:
    Serial.printf("unknown packet type: %hhu\n", packet.type);
    reply((void*)"unknown", strlen("unknown"));
    break;
  }
}

// TODO: test
MultiCom::session *MultiCom::_getSession(u32_t id) {

  for (MultiCom::session *s : _session_list) {
    if (s->id == id) return s;
  }

  // not found -> add new
  session *new_session = (session *) malloc(sizeof(session));
  *new_session = {id, 0};
  _session_list.push_back(new_session);
  
  // when an overflow occurs, remove oldest
  if (_session_list.size() > MAX_CONCURRENT_SESSIONS) {
    session *removed = _session_list.front();
    _session_list.pop_front();
    free(removed);
  }

  return new_session;
}

