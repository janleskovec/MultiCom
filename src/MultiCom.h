#ifndef multicom_h
#define multicom_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>

#include "MultiComPacket.h"


/*
*   session struct
*/
typedef struct MultiComSession {
  u32_t id;
  u32_t nonce;
  u32_t last_ack_nonce;
} MultiComSession;

typedef std::function<void(void *data, u16_t len)> MultiComReplyFn;
typedef std::function<void(void *data, u16_t len, MultiComReplyFn reply)> MultiComNewDataFn;

class MultiComChannel {

  public:
    virtual bool start();
    virtual void stop();
    bool isRunning;

    // is set by MultiCom
    MultiComNewDataFn _callback;
};

typedef std::function<void(MultiComPacket packet, MultiComReplyFn reply)> MultiComEndpointCallback;

class MultiCom {

  public:
    MultiCom(MultiComChannel *udp);
    
    bool startAll();

    // TODO: for testing only, remove!
    MultiComEndpointCallback tmp_callback;

    MultiComChannel *channelUdp;
    //MultiComChannel *channelBle;
  
  private:
    void _endpointRouter(MultiComPacket packet, MultiComReplyFn reply);
    void _onNewMsg(void *data, u16_t len, MultiComReplyFn reply);

};

#endif
