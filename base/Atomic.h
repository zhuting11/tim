#ifndef TIM_BASE_ATOMIC_H
#define TIM_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tim
{

namespace detail
{
class AtomicInteger : boost::noncopyable
{
 public:
  AtomicInteger()
    : value_(0)
  {
  }

  LONG get()
  {
	 return value_;
  }

  LONG getAndAdd(LONG x)
  {
	return InterlockedExchangeAdd(&value_, x);
  }

  LONG addAndGet(LONG x)
  {
    return getAndAdd(x) + x;
  }

  LONG incrementAndGet()
  {
    return addAndGet(1);
  }

  LONG decrementAndGet()
  {
    return addAndGet(-1);
  }

  void add(LONG x)
  {
    getAndAdd(x);
  }

  void increment()
  {
    incrementAndGet();
  }

  void decrement()
  {
    decrementAndGet();
  }

  LONG getAndSet( LONG newValue)
  {
	return InterlockedExchange(&value_, newValue);
  }

 private:
  volatile LONG value_;
};
}

typedef detail::AtomicInteger AtomicInt32;
}

#endif  // TIM_BASE_ATOMIC_H
