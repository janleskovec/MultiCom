#ifndef multicom_h
#define multicom_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>


/*typedef std::function<void(MultiComPacket& packet)> MultiComCallback;
class MultiComEndpoint {

  public:
    MultiComEndpoint(char *endpoint, MultiComCallback callback);

  private:
    char *_endpoint;
    MultiComCallback callback;
};*/


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

class MultiCom {

  public:
    MultiCom(MultiComChannel *udp);
    
    bool startAll();

    MultiComChannel *channelUdp;
    //MultiComChannel *channelBle;
  
  private:
    void _onNewMsg(void *data, u16_t len, MultiComReplyFn reply);

};

#endif
