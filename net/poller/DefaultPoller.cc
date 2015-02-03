#include <tim/net/Poller.h>
#include <tim/net/poller/PollPoller.h>
//#include <tim/net/poller/EPollPoller.h>

#include <stdlib.h>

using namespace tim::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  //if (::getenv("MUDUO_USE_POLL"))
  //{
    return new PollPoller(loop);
  //}
  //else
  //{
  //  return new EPollPoller(loop);
  //}
}
