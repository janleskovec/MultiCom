#ifndef multicom_h
#define multicom_h

#include <Arduino.h>
#include <lwip/udp.h>
#include <functional>

typedef void(* MultiComReplyFn) (void *, u16_t);


/*typedef std::function<void(MultiComPacket& packet)> MultiComCallback;
class MultiComEndpoint {

  public:
    MultiComEndpoint(char *endpoint, MultiComCallback callback);

  private:
    char *_endpoint;
    MultiComCallback callback;
};*/

class MultiComMsg {

  public:
    MultiComMsg(void *n_data, u16_t n_len, MultiComReplyFn *n_reply);
    void *data;
    u16_t len;

    // callback
    MultiComReplyFn *reply;
};

typedef void(* MultiComNewDataFn) (MultiComMsg *);

class MultiComChannel {

  public:
    virtual bool start();
    virtual void stop();
    bool isRunning;

    // is set by MultiCom
    MultiComNewDataFn *_callback;
};

class MultiCom {

  public:
    MultiCom(MultiComChannel *udp);
    
    bool startAll();

    MultiComChannel *channelUdp;
    //MultiComChannel *channelBle;
  
  private:
    void _onNewMsg(MultiComMsg *msg);

};

#endif
