#ifndef TIM_BASE_CONDITION_H
#define TIM_BASE_CONDITION_H

#include <tim/base/Mutex.h>

#include <boost/noncopyable.hpp>
//#include <pthread.h>

namespace tim
{

	typedef CONDITION_VARIABLE pthread_cond_t;
	typedef int pthread_condattr_t;
#define PTHREAD_COND_INITIALIZER {0}

struct timespec
{
	/* long long in windows is the same as long in unix for 64bit */
	long long tv_sec;
	long long tv_nsec;
};

//static unsigned long long _pthread_time_in_ms(void)
//{
//	struct __timeb64 tb;
//	
//	_ftime64(&tb);
//	
//	return tb.time * 1000 + tb.millitm;
//}

static unsigned long long _pthread_time_in_ms_from_timespec(const struct timespec *ts)
{
	unsigned long long t = ts->tv_sec * 1000;
	t += ts->tv_nsec / 1000000;

	return t;
}

//static unsigned long long _pthread_rel_time_in_ms(const struct timespec *ts)
//{
//	unsigned long long t1 = _pthread_time_in_ms_from_timespec(ts);
//	unsigned long long t2 = _pthread_time_in_ms();
//	
//	/* Prevent underflow */
//	if (t1 < t2) return 1;
//	return t1 - t2;
//}

static int pthread_cond_init(pthread_cond_t *c, pthread_condattr_t *a)
{
	(void) a;
	
	InitializeConditionVariable(c);
	return 0;
}

static int pthread_cond_signal(pthread_cond_t *c)
{
	WakeConditionVariable(c);
	return 0;
}

static int pthread_cond_broadcast(pthread_cond_t *c)
{
	WakeAllConditionVariable(c);
	return 0;
}

static int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
	//pthread_testcancel();
	SleepConditionVariableCS(c, m, INFINITE);
	return 0;
}

static int pthread_cond_destroy(pthread_cond_t *c)
{
	(void) c;
	return 0;
}

static int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, struct timespec *t)
{
	//unsigned long long tm = _pthread_rel_time_in_ms(t);
	unsigned long long tm = _pthread_time_in_ms_from_timespec(t);
	
	
	//pthread_testcancel();
	
	if (!SleepConditionVariableCS(c, m, tm)) return ETIMEDOUT_TIM;
	
	/* We can have a spurious wakeup after the timeout */
	//if (!_pthread_rel_time_in_ms(t)) return ETIMEDOUT;
	
	return 0;
}

class Condition : boost::noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    MCHECK(pthread_cond_init(&pcond_, NULL));
  }

  ~Condition()
  {
    MCHECK(pthread_cond_destroy(&pcond_));
  }

  void wait()
  {
    MutexLock::UnassignGuard ug(mutex_);
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(int seconds);

  void notify()
  {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll()
  {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};

}
#endif  // MUDUO_BASE_CONDITION_H
