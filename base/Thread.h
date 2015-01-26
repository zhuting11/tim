#ifndef TIM_BASE_THREAD_H
#define TIM_BASE_THREAD_H

#include <tim/base/Atomic.h>
#include <tim/base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
//#include <pthread.h>

namespace tim
{

class Thread : boost::noncopyable
{
 public:
  typedef boost::function<void ()> ThreadFunc;

  explicit Thread(const ThreadFunc&, const string& name = string());
//#ifdef __GXX_EXPERIMENTAL_CXX0X__
//  explicit Thread(ThreadFunc&&, const string& name = string());
//#endif
  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  DWORD tid() const { return *tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;
  bool       joined_;
  //pthread_t  pthreadId_;	 //pthreadId_(pthread_t)  like thread_(HANDLE) in windows
  //boost::shared_ptr<pid_t> tid_;	//like threadID_ in windows
  boost::shared_ptr<DWORD> tid_;
  ThreadFunc func_;
  string     name_;

  static AtomicInt32 numCreated_;

  HANDLE     thread_;
  //DWORD		 threadId_; // use tid_
};

}
#endif
