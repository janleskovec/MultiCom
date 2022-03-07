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

  // NOTE: for debugging only (adds significant delay)
  /*uint32_t addr4 = addr->u_addr.ip4.addr;
  Serial.printf("Received %d bytes from: ", p->tot_len);
  Serial.printf("%d",  ( addr4 & 0x000000ff ) >>  0);
  Serial.printf(".%d", ( addr4 & 0x0000ff00 ) >>  8);
  Serial.printf(".%d", ( addr4 & 0x00ff0000 ) >> 16);
  Serial.printf(".%d", ( addr4 & 0xff000000 ) >> 24);
  Serial.print("\n");*/

  using namespace std::placeholders;
  MultiComReplyFn _reply = std::bind(&MultiComUdp::_send, this, addr, ntohs(port), _1, _2);

  // iteration through packet chain
  while(p != NULL) {
    if (_callback != NULL)
      _callback(
        p->payload,
        p->len,
        _reply
      );
    
    pbuf * tmp = p;
    p = p->next;
    tmp->next = NULL;
    pbuf_free(tmp);
  }
}


void MultiComUdp::_send(const ip_addr_t * ip, u16_t port, void *data, u16_t len) {

  // limit len
  if (len > 1460) len = 1460;

  // generate packet buffer
  struct pbuf *p;
  p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
  memcpy (p->payload, data, len);

  // send, check err
  if (udp_sendto(this->_pcb, p, ip, htons(port))) Serial.println("Err sending udp packet");
  
  // free buff
  pbuf_free(p);
}
