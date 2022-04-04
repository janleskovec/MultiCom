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
    bool isRunning = false;

    // is set by MultiCom
    MultiComNewDataFn _callback;
};


// user-facing-callbacks
typedef std::function<void(MultiComPacket packet, MultiComReplyFn reply)> MultiComGetCallback;
typedef std::function<void(MultiComPacket packet)> MultiComSendCallback;
typedef std::function<void(MultiComPacket packet)> MultiComPostCallback;


class MultiComGetEndpoint {

  public:
    const char *endpoint;
    MultiComGetCallback callback;

    void setCallback(MultiComGetCallback c) { callback = c; }
};

class MultiComSendEndpoint {

  public:
    const char *endpoint;
    MultiComSendCallback callback;

    void setCallback(MultiComSendCallback c) { callback = c; }
};

class MultiComPostEndpoint {

  public:
    const char *endpoint;
    MultiComPostCallback callback;

    void setCallback(MultiComPostCallback c) { callback = c; }
};

/*
*   main wrapper
*/
#define MAX_CONCURRENT_SESSIONS 16
class MultiCom {

  public:
    MultiCom(MultiComChannel *udp, MultiComChannel *ble);
    
    bool startAll();

    MultiComChannel *channelUdp;
    MultiComChannel *channelBle;

    void setDiscoveryResponse(const char *fwId, const char *devId, u32_t ver);

    void onGet(const char *name, MultiComGetCallback callback);
    void onSend(const char *name, MultiComSendCallback callback);
    void onPost(const char *name, MultiComPostCallback callback);
  
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

    std::list<MultiComGetEndpoint*> _get_endpoints;
    std::list<MultiComSendEndpoint*> _send_endpoints;
    std::list<MultiComPostEndpoint*> _post_endpoints;

    MultiComGetCallback _getGetCallback(const char *name);
    MultiComSendCallback _getSendCallback(const char *name);
    MultiComPostCallback _getPostCallback(const char *name);

};

#endif
