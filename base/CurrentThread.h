#ifndef TIM_BASE_CURRENTTHREAD_H
#define TIM_BASE_CURRENTTHREAD_H

#include <stdint.h>

namespace tim
{
namespace CurrentThread
{
  // internal
  extern __declspec(thread) int t_cachedTid;
  extern __declspec(thread) char t_tidString[32];
  extern __declspec(thread) int t_tidStringLength;
  extern __declspec(thread) const char* t_threadName;
  void cacheTid();

  inline int tid()
  {
    //if (__builtin_expect(t_cachedTid == 0, 0))
	if (t_cachedTid == 0)
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread();

  //void sleepUsec(int64_t usec);
  void sleepMsec(int64_t msec);
}
}

#endif
