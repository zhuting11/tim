#ifndef TIM_NET_TIMERID_H
#define TIM_NET_TIMERID_H

#include <tim/base/copyable.h>

namespace tim
{
namespace net
{

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public tim::copyable
{
 public:
  TimerId()
    : timer_(NULL),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }

  // default copy-ctor, dtor and assignment are okay

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};

}
}

#endif  // TIM_NET_TIMERID_H
