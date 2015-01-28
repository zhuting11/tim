#ifndef TIM_BASE_COUNTDOWNLATCH_H
#define TIM_BASE_COUNTDOWNLATCH_H

#include <tim/base/Condition.h>
#include <tim/base/Mutex.h>

#include <boost/noncopyable.hpp>

namespace tim
{

class CountDownLatch : boost::noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}
#endif  // TIM_BASE_COUNTDOWNLATCH_H
