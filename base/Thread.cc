#include <tim/base/Thread.h>
#include <tim/base/CurrentThread.h>
#include <tim/base/Exception.h>
#include <tim/base/Logging.h>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>

#include <process.h>

#include <errno.h>
#include <stdio.h>
//#include <unistd.h>
//#include <sys/prctl.h>
//#include <sys/syscall.h>
//#include <sys/types.h>
//#include <linux/unistd.h>

namespace tim
{
namespace CurrentThread
{
  __declspec(thread) int t_cachedTid = 0;
  __declspec(thread) char t_tidString[32];
  __declspec(thread) int t_tidStringLength = 6;
  __declspec(thread) const char* t_threadName = "unknown";
  //const bool sameType = boost::is_same<int, pid_t>::value;
  //BOOST_STATIC_ASSERT(sameType);
}

namespace detail
{

DWORD gettid()
{
  //return static_cast<pid_t>(::syscall(SYS_gettid));

  return GetCurrentThreadId();
}


//
// Usage: SetThreadName (-1, "MainThread");
//
#define MS_VC_EXCEPTION 0x406D1388

typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = szThreadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
   }
   __except(EXCEPTION_CONTINUE_EXECUTION)
   {
   }
}

//void afterFork()
//{
//  muduo::CurrentThread::t_cachedTid = 0;
//  muduo::CurrentThread::t_threadName = "main";
//  CurrentThread::tid();
//  // no need to call pthread_atfork(NULL, NULL, &afterFork);
//}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    tim::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    //pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData
{
  typedef tim::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  boost::weak_ptr<DWORD> wkTid_;

  ThreadData(const ThreadFunc& func,
             const string& name,
             const boost::shared_ptr<DWORD>& tid)
    : func_(func),
      name_(name),
      wkTid_(tid)
  { }

  void runInThread()
  {
    DWORD tid = tim::CurrentThread::tid();

    boost::shared_ptr<DWORD> ptid = wkTid_.lock();
    if (ptid)
    {
      *ptid = tid;
      ptid.reset();
    }

    tim::CurrentThread::t_threadName = name_.empty() ? "timThread" : name_.c_str();
    //::prctl(PR_SET_NAME, tim::CurrentThread::t_threadName);
	detail::SetThreadName(-1, tim::CurrentThread::t_threadName);
    try
    {
      func_();
      tim::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      tim::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      tim::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      tim::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
};

unsigned __stdcall  startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

}
}

using namespace tim;

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = _snprintf_s(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

//bool CurrentThread::isMainThread()
//{
//  return tid() == ::getpid();
//}

//void CurrentThread::sleepUsec(int64_t usec)
//{
//  struct timespec ts = { 0, 0 };
//  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
//  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
//  ::nanosleep(&ts, NULL);
//}

void CurrentThread::sleepMsec(int64_t msec)
{
  //struct timespec ts = { 0, 0 };
  //ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  //ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  //::nanosleep(&ts, NULL);
  ::Sleep(msec);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
  : started_(false),
    joined_(false),
	thread_(NULL),
    tid_(new DWORD(0)),
    func_(func),
    name_(n)
{
  setDefaultName();
}

//#ifdef __GXX_EXPERIMENTAL_CXX0X__
//Thread::Thread(ThreadFunc&& func, const string& n)
//  : started_(false),
//    joined_(false),
//    pthreadId_(0),
//    tid_(new pid_t(0)),
//    func_(std::move(func)),
//    name_(n)
//{
//  setDefaultName();
//}
//
//#endif

Thread::~Thread()
{
  if (started_ && !joined_ && thread_)
  {
    //pthread_detach(pthreadId_);
	  CloseHandle(thread_);
  }
}

void Thread::setDefaultName()
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    _snprintf_s(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
  //if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
  unsigned threadId;
  thread_ = (HANDLE) _beginthreadex(NULL, NULL, &detail::startThread, data, 0, &threadId);
  *tid_ = static_cast<DWORD>(threadId);

  if (!thread_)
  {
    started_ = false;
    delete data; // or no delete?
    LOG_SYSFATAL << "Failed in pthread_create";
  }
}

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	if (!thread_) return 0;
	joined_ = true;
	//return pthread_join(pthreadId_, NULL);

	switch (WaitForSingleObject(thread_, INFINITE))
	{
	case WAIT_OBJECT_0:
		if (!thread_) return 0;
		if (CloseHandle(thread_)) thread_ = 0;
		return 0;
	default:
		LOG_SYSFATAL << "Cannot join thread";
		return -1;
	}
}

