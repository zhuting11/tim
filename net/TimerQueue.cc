#define __STDC_LIMIT_MACROS
#include <tim/net/TimerQueue.h>

#include <tim/base/Logging.h>
#include <tim/net/EventLoop.h>
#include <tim/net/Timer.h>
#include <tim/net/TimerId.h>
#include <tim/base/Condition.h>

#include <boost/bind.hpp>
#include <tim/net/socketsops.h>

//#include <sys/timerfd.h>

namespace tim
{
namespace net
{
namespace detail
{
	//typedef int ssize_t;

//int createTimerfd()
//{
//  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
//                                 TFD_NONBLOCK | TFD_CLOEXEC);
//  if (timerfd < 0)
//  {
//    LOG_SYSFATAL << "Failed in timerfd_create";
//  }
//  return timerfd;
//}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

//void readTimerfd(int timerfd, Timestamp now)
//{
//  uint64_t howmany;
//  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
//  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
//  if (n != sizeof howmany)
//  {
//    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
//  }
//}

//void resetTimerfd(int timerfd, Timestamp expiration)
//{
//  // wake up loop by timerfd_settime()
//  struct itimerspec newValue;
//  struct itimerspec oldValue;
//  bzero(&newValue, sizeof newValue);
//  bzero(&oldValue, sizeof oldValue);
//  newValue.it_value = howMuchTimeFromNow(expiration);
//  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
//  if (ret)
//  {
//    LOG_SYSERR << "timerfd_settime()";
//  }
//}

}
}
}

using namespace tim;
using namespace tim::net;
using namespace tim::net::detail;
using namespace tim::net::sockets;

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
  	hTimer_(NULL),	// by tim
	timerFdSock_(creatSockPair()),  //add by tim
    //timerfd_(createTimerfd()),
    //timerfdChannel_(loop, timerfd_),
	timerfdChannel_(loop, timerFdSock_.first),
    timers_(),
    callingExpiredTimers_(false)

{
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  //::close(timerfd_);
  closesocket(timerFdSock_.first);
  closesocket(timerFdSock_.second);

  timerFdSock_.first = NULL;
  timerFdSock_.second = NULL;

  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin();
      it != timers_.end(); ++it)
  {
    delete it->second;
  }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(cb, when, interval);
  loop_->runInLoop(
      boost::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
TimerId TimerQueue::addTimer(TimerCallback&& cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(
      boost::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}
#endif

void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
      boost::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged)
  {
    //resetTimerfd(timerfd_, timer->expiration());
	resetTimerEx(timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  ActiveTimerSet::iterator it = activeTimers_.find(timer);
  if (it != activeTimers_.end())
  {
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1); (void)n;
    delete it->first; // FIXME: no delete please
    activeTimers_.erase(it);
  }
  else if (callingExpiredTimers_)
  {
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());

  ////edit by tim
  //readTimerfd(timerfd_, now);
  uint64_t one = 1;
  ssize_t n = sockets::read(timerFdSock_.first, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }

  std::vector<Entry> expired = getExpired(now);

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();
  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    it->second->run();
  }
  callingExpiredTimers_ = false;

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator end = timers_.lower_bound(sentry);
  assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);

  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    ActiveTimer timer(it->second, it->second->sequence());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1); (void)n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for (std::vector<Entry>::const_iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    ActiveTimer timer(it->second, it->second->sequence());
    if (it->second->repeat()
        && cancelingTimers_.find(timer) == cancelingTimers_.end())
    {
      it->second->restart(now);
      insert(it->second);
    }
    else
    {
      // FIXME move to a free list
      delete it->second; // FIXME: no delete please
    }
  }

  if (!timers_.empty())
  {
    nextExpire = timers_.begin()->second->expiration();
  }

  if (nextExpire.valid())
  {
	//edit by tim
    //resetTimerfd(timerfd_, nextExpire);
	resetTimerEx(nextExpire);
  }
}

bool TimerQueue::insert(Timer* timer)
{
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first)
  {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result
      = timers_.insert(Entry(when, timer));
    assert(result.second); (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result
      = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second); (void)result;
  }

  assert(timers_.size() == activeTimers_.size());
  return earliestChanged;
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if(lpParam == NULL)
	{
		LOG_WARN << "TimerRoutine get NULL lpParam";
		return;
	}

	SOCKET sk = *(SOCKET*)(lpParam);

	if(sk == INVALID_SOCKET)
	{
		LOG_WARN << "TimerRoutine get INVALID_SOCKET";
		return;
	}

	uint64_t one = 1;
	ssize_t n = sockets::write(sk, &one, sizeof one);
	if (n != sizeof one)
	{
		LOG_WARN << "TimerRoutine writes " << n << " bytes instead of 8";
	}
}

//by zhuting(tim)
void TimerQueue::resetTimerEx(Timestamp expiration)
{
	int64_t microseconds = expiration.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();

	DWORD dueTim = microseconds/1000;

	if(hTimer_)
	{
		if(!DeleteTimerQueueTimer(NULL, hTimer_, NULL))
			LOG_SYSERR << "DeleteTimerQueueTimer failed! GetLastError = " << GetLastError();
		hTimer_ = NULL;
	}

    // Set a timer to call the timer routine in 10 seconds.
    if (!CreateTimerQueueTimer( &hTimer_, NULL, 
		(WAITORTIMERCALLBACK)TimerRoutine, &timerFdSock_.second , dueTim, 0, 0))
    {
		LOG_SYSERR << "CreateTimerQueueTimer failed! GetLastError = " << GetLastError();
    }
}

