#include <tim/net/Timer.h>

using namespace tim;
using namespace tim::net;

//AtomicInt64 Timer::s_numCreated_;
AtomicInt32 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}
