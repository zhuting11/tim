#ifndef TIM_BASE_ASYNCLOGGING_H
#define TIM_BASE_ASYNCLOGGING_H

#include <tim/base/BlockingQueue.h>
#include <tim/base/BoundedBlockingQueue.h>
#include <tim/base/CountDownLatch.h>
#include <tim/base/Mutex.h>
#include <tim/base/Thread.h>

#include <tim/base/LogStream.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace tim
{

class AsyncLogging : boost::noncopyable
{
 public:

  AsyncLogging(const string& basename,
               size_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  // declare but not define, prevent compiler-synthesized functions
  AsyncLogging(const AsyncLogging&);  // ptr_container
  void operator=(const AsyncLogging&);  // ptr_container

  void threadFunc();

  typedef tim::detail::FixedBuffer<tim::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  string basename_;
  size_t rollSize_;
  tim::Thread thread_;
  tim::CountDownLatch latch_;
  tim::MutexLock mutex_;
  tim::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}
#endif  // TIM_BASE_ASYNCLOGGING_H
