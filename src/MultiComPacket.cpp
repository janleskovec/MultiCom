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
            if (type != MultiComPacket::packet_type::discovery      &&
                type != MultiComPacket::packet_type::discovery_helo
                ) {
                    // session id
                    session_id = ntohl(*((u32_t *) data));
                    data = static_cast<char*>(data) +  sizeof(u32_t);
                    parsed_bytes += sizeof(u32_t);
            
                    // nonce
                    nonce = ntohl(*((u32_t *) data));
                    data = static_cast<char*>(data) +  sizeof(u32_t);
                    parsed_bytes += sizeof(u32_t);
                }
        }

        // endpoint
        if (len - parsed_bytes >= 0 &&
            (type != MultiComPacket::packet_type::get  ||
             type != MultiComPacket::packet_type::send ||
             type != MultiComPacket::packet_type::post
            )) {
                u16_t e_len = strlen((char *) data)+1;
                endpoint = (char*) data;
                data = static_cast<char*>(data) + e_len;
                parsed_bytes += sizeof(e_len);
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

// \0<fw_id>\0<dev_id>\0<api_ver_int>
MultiComPacket MultiComPacket::genDiscoveryReply(const char *fw_id, const char *dev_id, u32_t api_ver) {
    // to string
    char api_ver_str[16];
    sprintf(api_ver_str, "%zu", api_ver);

    // get length, allocate memory
    size_t len = sizeof(u8_t) + strlen(dev_id)+1 + strlen(fw_id)+1 + strlen(api_ver_str)+1;
    char *msg_data = (char *) malloc(len);
    
    // fill data
    char *tmp = msg_data;
    *tmp = (char) MultiComPacket::packet_type::discovery_helo;
    tmp += sizeof(u8_t);
    strcpy(tmp, fw_id);
    tmp += strlen(tmp)+1;
    strcpy(tmp, dev_id);
    tmp += strlen(tmp)+1;
    strcpy(tmp, api_ver_str);
    tmp += strlen(tmp)+1;
    
    return MultiComPacket(msg_data, len);
}

MultiComPacket MultiComPacket::genGetReply(MultiComPacket request, void *data, u16_t len) {
    u16_t tot_len = sizeof(u8_t) + (2*sizeof(u32_t)) + len;
    char *msg_data = (char*) malloc(tot_len);
    
    char *tmp = msg_data;
    *tmp = (char) MultiComPacket::packet_type::get_reply;
    tmp += sizeof(u8_t);
    *((u32_t*)tmp) = htonl(request.session_id);
    tmp += sizeof(u32_t);
    *((u32_t*)tmp) = htonl(request.nonce);
    tmp += sizeof(u32_t);
    memcpy(tmp, data, len);
    tmp += len;

    return MultiComPacket(msg_data, tot_len);
}
