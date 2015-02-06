#ifndef TIM_NET_POLLER_POLLPOLLER_H
#define TIM_NET_POLLER_POLLPOLLER_H

#include <tim/net/Poller.h>

#include <vector>

//struct pollfd;

namespace tim
{
namespace net
{

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller
{
 public:

  PollPoller(EventLoop* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;

  //typedef std::vector<struct pollfd> PollFdList;
  typedef std::vector<WSAPOLLFD> PollFdList;
  PollFdList pollfds_;
};

}
}
#endif  // TIM_NET_POLLER_POLLPOLLER_H
