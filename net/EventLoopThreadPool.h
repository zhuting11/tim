#ifndef TIM_NET_EVENTLOOPTHREADPOOL_H
#define TIM_NET_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace tim
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());
  // valid after calling start()
  EventLoop* getNextLoop();
  // valid after calling start()
  std::vector<EventLoop*> getAllLoops();

 private:

  EventLoop* baseLoop_;
  bool started_;
  int numThreads_;
  int next_;
  boost::ptr_vector<EventLoopThread> threads_;
  std::vector<EventLoop*> loops_;
};

}
}

#endif  // TIM_NET_EVENTLOOPTHREADPOOL_H
