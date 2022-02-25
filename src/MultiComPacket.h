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
      discovery  = 0, // discovery packet (only returns identifier msg)
      get        = 1, // does not use nonce (callback called again on re-transmit)
      send       = 2, // uses nonce (ensures callback only gets called once + order)
      post       = 3, // uses nonce + sends ack (ensures callback only gets called once + order)
      ack        = 4, // used to reply after post (contains session id and latest nonce)
    };

    MultiComPacket(void *data, u16_t len);

    // enum packet_type
    packet_type type;
    // session info
    u32_t session_id;
    u32_t nonce;

    // points to _raw_data, where non-header data starts
    void *user_data;
    u16_t user_len;

    static MultiComPacket genAckPacket(u32_t session_id, u32_t nonce);

    void *_raw_data;
    u16_t _raw_len;
};

#endif
