#include <Arduino.h>
#include <lwip/udp.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <functional>

#include "MultiCom.h"
#include "MultiComPacket.h"

// read received packet
MultiComPacket::MultiComPacket(void *data, u16_t len) {
    _raw_data = data;
    _raw_len = len;

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
                type == MultiComPacket::packet_type::send ||
                type == MultiComPacket::packet_type::post ||
                type == MultiComPacket::packet_type::ack
                ) {
                    session_id = ntohl(*((u32_t *) data));
                    data = static_cast<char*>(data) +  sizeof(u32_t);
                    parsed_bytes += sizeof(u32_t);
                }
            
            // nonce
            if (type == MultiComPacket::packet_type::send ||
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


MultiComPacket MultiComPacket::genAckPacket( u32_t session_id, u32_t nonce) {
    char *ack_data = (char*) malloc(sizeof(u8_t) + 2*sizeof(u32_t));
    
    char *tmp = ack_data;
    *tmp = (char) MultiComPacket::packet_type::ack;
    tmp += sizeof(u8_t);
    *((u32_t*)tmp) = htonl(session_id);
    tmp += sizeof(u32_t);
    *((u32_t*)tmp) = htonl(nonce);
    tmp += sizeof(u32_t);
    
    return MultiComPacket(ack_data, (static_cast<char*>(ack_data) - static_cast<char*>(tmp)));
}

MultiComPacket MultiComPacket::genDiscoveryReply(char *msg) {
    char *msg_data = (char*) malloc(sizeof(u8_t) + strlen(msg)+1);
    
    char *tmp = msg_data;
    *tmp = (char) MultiComPacket::packet_type::discovery;
    tmp += sizeof(u8_t);
    strcpy((char*) tmp, msg);
    
    return MultiComPacket(msg_data, (static_cast<char*>(msg_data) - static_cast<char*>(tmp)));
}
