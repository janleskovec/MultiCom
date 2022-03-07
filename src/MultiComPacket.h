#ifndef multicom_packet_h
#define multicom_packet_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>


/*
*   data packet wrapper
*/
class MultiComPacket {

  public:
    enum class packet_type : u8_t {
      discovery       = 0, // discovery packet (only returns identifier msg)
      discovery_helo  = 1, // discovery reply packet
      ping            = 2, // echo
      get             = 3, // uses random nonce (does not prevent dupicate callbacks)
      get_reply       = 4, // reply msg after get
      send            = 5, // uses sequential nonce (ensures callback only gets called once + order)
      post            = 6, // uses sequential nonce + sends ack (ensures callback only gets called once + order)
      ack             = 7, // used to reply after post (contains session id and nonce)
      not_found       = 8, // endpoint not found (contains session id and nonce)
    };

    MultiComPacket(void *data, u16_t len);

    // enum packet_type
    packet_type type;
    // session info
    u32_t session_id;
    u32_t nonce;

    // get, send, post
    char *endpoint;

    // points to _raw_data, where non-header data starts
    void *user_data;
    u16_t user_len;

    static MultiComPacket genAckPacket(u32_t session_id, u32_t nonce);
    static MultiComPacket genDiscoveryReply(const char *fw_id, const char *dev_id, u32_t api_ver);
    static MultiComPacket genGetReply(MultiComPacket request, void *data, u16_t len);
    static MultiComPacket genNotFoundReply(MultiComPacket request);

    void *_raw_data;
    u16_t _raw_len;
};

#endif
