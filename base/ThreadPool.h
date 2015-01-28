#ifndef TIM_BASE_THREADPOOL_H
#define TIM_BASE_THREADPOOL_H

#include <tim/base/Condition.h>
#include <tim/base/Mutex.h>
#include <tim/base/Thread.h>
#include <tim/base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

namespace tim
{

class ThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void ()> Task;

  explicit ThreadPool(const string& name = string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
  void setThreadInitCallback(const Task& cb)
  { threadInitCallback_ = cb; }

  void start(int numThreads);
  void stop();

  // Could block if maxQueueSize > 0
  void run(const Task& f);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void run(Task&& f);
#endif

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  MutexLock mutex_;
  Condition notEmpty_;
  Condition notFull_;
  string name_;
  Task threadInitCallback_;
  boost::ptr_vector<tim::Thread> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
};

}

#endif
