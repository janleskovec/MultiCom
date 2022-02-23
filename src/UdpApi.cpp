#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>

#include "UdpApi.h"


UdpApi::UdpApi(u16_t port) {
  _port = port;
}

bool UdpApi::start() {
  bool success = true;

  Serial.println("start udp server");

  _pcb = udp_new();
  if(_pcb == NULL) {
    close();
    return false;
  }
  if (udp_bind(_pcb, IP_ADDR_ANY, _port)) {
    close();
    return false;
  }

  udp_recv( _pcb, &_udp_receive_callback, (void *) this);

  return success;
}

void UdpApi::close() {
  Serial.println("stop udp server");

  udp_disconnect(_pcb);
  udp_remove(_pcb);
}


// inspitarion: https://github.com/me-no-dev/ESPAsyncUDP/blob/master/src/AsyncUDP.cpp

// binder function to gain access to class instance
void UdpApi::_udp_receive_callback( void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port ) {
  reinterpret_cast<UdpApi*>(arg)->_udp_receive_callback(upcb, p, (ip_addr_t *)addr, port);
}

// new packet callback function
void UdpApi::_udp_receive_callback(
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
  Serial.print("Data: ");
  for (int i = 0; i < p->len; i++) {
    Serial.printf("%4d ", ((char*) p->payload)[i]);
  }
  Serial.print("\n      ");
  for (int i = 0; i < p->len; i++) {
    Serial.printf("   %c ", ((char*) p->payload)[i]);
  }
  Serial.println();
  
  // free receive pbuf
  pbuf_free(p);
}

