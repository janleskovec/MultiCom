#ifndef multicom_h
#define multicom_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>
#include <list>

#include "MultiComPacket.h"

// reply callback (backend facing)
typedef std::function<void(void *data, u16_t len)> MultiComReplyFn;
// wrapper callback (backend -> wrapper)
typedef std::function<void(void *data, u16_t len, MultiComReplyFn reply)> MultiComNewDataFn;


/*
*   comm channels (backends)
*/
class MultiComChannel {

  public:
    virtual bool start();
    virtual void stop();
    bool isRunning;

    // is set by MultiCom
    MultiComNewDataFn _callback;
};


// user-facing-callback
typedef std::function<void(MultiComPacket packet, MultiComReplyFn reply)> MultiComEndpointCallback;
typedef std::function<char*(void)> MultiComDiscoveryCallback;


/*
*   main wrapper
*/
#define MAX_CONCURRENT_SESSIONS 16
class MultiCom {

  public:
    MultiCom(MultiComChannel *udp);
    
    bool startAll();

    // TODO: for testing only, remove!
    MultiComEndpointCallback tmp_callback;

    MultiComChannel *channelUdp;
    //MultiComChannel *channelBle;

    void setDiscoveryResponse(const char *fwId, const char *devId, u32_t ver);
  
  private:
    void *_discovery_response;
    u16_t _discovery_response_len;

    void _endpointRouter(MultiComPacket packet, MultiComReplyFn reply);
    void _onNewMsg(void *data, u16_t len, MultiComReplyFn reply);

    typedef struct session {
      u32_t id;
      u32_t nonce;
    } session;

    std::list<session*> _session_list;

    session *_getSession(u32_t id);

};

#endif
