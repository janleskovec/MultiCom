#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>

#include "MultiCom.h"
#include "MultiComUdp.h"


MultiComUdp::MultiComUdp(u16_t port) {
  _port = port;
}

bool MultiComUdp::start() {
  bool success = true;

  Serial.println("start udp server");

  _pcb = udp_new();
  if(_pcb == NULL) {
    stop();
    return false;
  }
  if (udp_bind(_pcb, IP_ADDR_ANY, _port)) {
    stop();
    return false;
  }

  udp_recv( _pcb, &_udp_receive_callback, (void *) this);

  isRunning = success;

  return success;
}

void MultiComUdp::stop() {
  isRunning = false;
  Serial.println("stop udp server");

  udp_disconnect(_pcb);
  udp_remove(_pcb);
}


// inspitarion: https://github.com/me-no-dev/ESPAsyncUDP/blob/master/src/AsyncUDP.cpp

// binder function to gain access to class instance
void MultiComUdp::_udp_receive_callback( void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port ) {
  reinterpret_cast<MultiComUdp*>(arg)->_udp_receive_callback(upcb, p, (ip_addr_t *)addr, port);
}

// new packet callback function
void MultiComUdp::_udp_receive_callback(
                struct udp_pcb* upcb,   // Receiving Protocol Control Block
                struct pbuf* p,         // Pointer to Datagram
                const ip_addr_t* addr,  // Address of sender
                u16_t port )            // Sender port 
{

  uint32_t addr4 = addr->u_addr.ip4.addr;

  Serial.printf("Received %d bytes from: ", p->len);
  Serial.printf("%d",  ( addr4 & 0x000000ff ) >>  0);
  Serial.printf(".%d", ( addr4 & 0x0000ff00 ) >>  8);
  Serial.printf(".%d", ( addr4 & 0x00ff0000 ) >> 16);
  Serial.printf(".%d", ( addr4 & 0xff000000 ) >> 24);
  Serial.print("\n");

  // TODO: fix
  MultiComUdpReplyContext rplCtx(this, addr, port);

  /*MultiComMsg *msg = new MultiComMsg(
    p->payload,
    p->len,
    std::bind1st(rplCtx._send, rplCtx)
  );*/

  //if (_callback != NULL) (*_callback)(msg);
  
  // free msg for callback
  //delete packet;
  // free receive pbuf
  pbuf_free(p);
}

MultiComUdpReplyContext::MultiComUdpReplyContext( MultiComUdp *channel, const ip_addr_t * ip, u16_t port){
  _channel = channel;
  _ip = ip;
  _port = port;
}

void MultiComUdpReplyContext::_send(void *data, u16_t len) {
  // TODO: impmenent (https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html#gaa0e135a5958f1f0cc83cbeb609e18743)
  Serial.println("Sending udp packet");
}
