#ifndef multicom_udp_h
#define multicom_udp_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>

#include "MultiCom.h"


class MultiComUdp : public MultiComChannel {

  public:
    MultiComUdp(u16_t port);
    bool start();
    void stop();

  private:
    u16_t _port;
    udp_pcb *_pcb;
    bool _connected;
    static void _udp_receive_callback( void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port );
    void _udp_receive_callback( struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port );

    void _send(const ip_addr_t * ip, u16_t port, void *data, u16_t len);
};


#endif
