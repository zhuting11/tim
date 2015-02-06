#ifndef TIM_NET_EVENTLOOPTHREAD_H
#define TIM_NET_EVENTLOOPTHREAD_H

#include <tim/base/Condition.h>
#include <tim/base/Mutex.h>
#include <tim/base/Thread.h>

#include <boost/noncopyable.hpp>

namespace tim
{
namespace net
{

class EventLoop;

class EventLoopThread : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_;
};

}
}

#endif  // TIM_NET_EVENTLOOPTHREAD_H

