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

  bool not_found = false;

  switch (packet.type)
  {
  case MultiComPacket::packet_type::get:
    {
      MultiComGetCallback get_callback = _getGetCallback(packet.endpoint);
      if (get_callback != NULL)
        get_callback(packet, [packet, reply](void *data, u16_t len){
          MultiComPacket replyPacket = MultiComPacket::genGetReply(packet, data, len);
          reply(replyPacket._raw_data, replyPacket._raw_len);
          free(replyPacket._raw_data); // free buff
        });
      else not_found = true;
    }
    break;
  
  case MultiComPacket::packet_type::send:
    if (packet.nonce > session->nonce) {
      // increase nonce
      session->nonce = packet.nonce;

      {
        MultiComSendCallback send_callback = _getSendCallback(packet.endpoint);
        if (send_callback != NULL) send_callback(packet);
        else not_found = true;
      }
    }

    break;
  
  case MultiComPacket::packet_type::post:
    if (packet.nonce > session->nonce) {
      // increase nonce
      session->nonce = packet.nonce;

      MultiComPostCallback post_callback = _getPostCallback(packet.endpoint);
      if (post_callback != NULL) post_callback(packet);
      else not_found = true;
    }
    
    //TODO: test
    // send ack
    MultiComPacket ackPacket = MultiComPacket::genAckPacket(packet.session_id, packet.nonce);
    reply(ackPacket._raw_data, ackPacket._raw_len);
    free(ackPacket._raw_data); // free buff

    break;
  }

  if (not_found) {
    // endpoint not found
    MultiComPacket notFoundPacket = MultiComPacket::genNotFoundReply(packet);
    reply(notFoundPacket._raw_data, notFoundPacket._raw_len);
    free(notFoundPacket._raw_data); // free buff
  }
}

void MultiCom::_onNewMsg(void *data, u16_t len, MultiComReplyFn reply){
  MultiComPacket packet = MultiComPacket(data, len);

  switch (packet.type)
  {
  case MultiComPacket::packet_type::discovery:
    // generate response
    if (_discovery_response != NULL)
      reply(_discovery_response, _discovery_response_len);
    break;
  
  case MultiComPacket::packet_type::ping:
    reply(data, len); // echo
    break;
  
  case MultiComPacket::packet_type::get:
  case MultiComPacket::packet_type::send:
  case MultiComPacket::packet_type::post:
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

void MultiCom::setDiscoveryResponse(const char *fwId, const char *devId, u32_t ver) {
  if (_discovery_response != NULL) free(_discovery_response);
  MultiComPacket packet = MultiComPacket::genDiscoveryReply(fwId, devId, ver);
  _discovery_response = packet._raw_data;
  _discovery_response_len = packet._raw_len;
}


void MultiCom::onGet(const char *name, MultiComGetCallback callback) {
  MultiComGetEndpoint *endpoint = new MultiComGetEndpoint();
  endpoint->endpoint = name;
  endpoint->setCallback(callback);
  _get_endpoints.push_back(endpoint);
}

void MultiCom::onSend(const char *name, MultiComSendCallback callback) {
  MultiComSendEndpoint *endpoint = new MultiComSendEndpoint();
  endpoint->endpoint = name;
  endpoint->setCallback(callback);
  _send_endpoints.push_back(endpoint);
}

void MultiCom::onPost(const char *name, MultiComPostCallback callback) {
  MultiComPostEndpoint *endpoint = new MultiComPostEndpoint();
  endpoint->endpoint = name;
  endpoint->setCallback(callback);
  _post_endpoints.push_back(endpoint);
}


MultiComGetCallback MultiCom::_getGetCallback(const char *endpoint) {
  for (MultiComGetEndpoint *e : _get_endpoints) {
    if (strcmp(e->endpoint, endpoint) == 0) return e->callback;
  }
  // not found
  return NULL;
}

MultiComSendCallback MultiCom::_getSendCallback(const char *endpoint) {
  for (MultiComSendEndpoint *e : _send_endpoints) {
    if (strcmp(e->endpoint, endpoint) == 0) return e->callback;
  }
  // not found
  return NULL;
}

MultiComPostCallback MultiCom::_getPostCallback(const char *endpoint){
  for (MultiComPostEndpoint *e : _post_endpoints) {
    if (strcmp(e->endpoint, endpoint) == 0) return e->callback;
  }
  // not found
  return NULL;
}
