#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <functional>

#include "MultiComPacket.h"

// read received packet
MultiComPacket::MultiComPacket(void *data, u16_t len) {
    _in_data = data;
    _in_len = len;

    session_id = 0;
    nonce = 0;

    uint16_t parsed_bytes = 0;

    // check len
    if (len > 0) {

        // header
        type = *((packet_type *) data); data = static_cast<char*>(data) + sizeof(packet_type);
        parsed_bytes += sizeof(packet_type);

        // check len (2x u32_t)
        if (len - parsed_bytes >= 8) {
            // session id
            if (type == MultiComPacket::packet_type::get  ||
                type == MultiComPacket::packet_type::set  ||
                type == MultiComPacket::packet_type::post ||
                type == MultiComPacket::packet_type::ack
                ) {
                    session_id = ntohl(*((u32_t *) data));
                    data = static_cast<char*>(data) +  sizeof(u32_t);
                    parsed_bytes += sizeof(u32_t);
                }
            
            // nonce
            if (type == MultiComPacket::packet_type::set ||
                type == MultiComPacket::packet_type::post
                ) {
                    nonce = ntohl(*((u32_t *) data));
                    data = static_cast<char*>(data) +  sizeof(u32_t);
                    parsed_bytes += sizeof(u32_t);
                }
        }
    }
    
    // user data
    user_data = data;
    user_len = len - parsed_bytes;
}

