#ifndef udp_api_h
#define udp_api_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>

class UdpApiPacket {

  public:
    UdpApiPacket(char *endpoint, void *data, u16_t len);
    char *endpoint;
    void *data;
    u16_t *len;
};

typedef std::function<void(UdpApiPacket& packet)> UdpApiCallback;

class UdpApiEndpoint {

  public:
    UdpApiEndpoint(char *endpoint, UdpApiCallback callback);

  private:
    char *_endpoint;
    UdpApiCallback callback;
};

class UdpApi {

  public:
    UdpApi(u16_t port);
    bool start();
    void close();

  private:
    u16_t _port;
    udp_pcb *_pcb;
    bool _connected;
    static void _udp_receive_callback( void* arg, struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port );
    void _udp_receive_callback( struct udp_pcb* upcb, struct pbuf* p, const ip_addr_t* addr, u16_t port );
};

#endif
