#ifndef TIM_BASE_LOGFILE_H
#define TIM_BASE_LOGFILE_H

#include <tim/base/Mutex.h>
#include <tim/base/Types.h>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <deque>

namespace tim
{

namespace FileUtil
{
class AppendFile;
}

class LogFile : boost::noncopyable
{
 public:
  LogFile(const string& basename,
          size_t rollSize,
		  size_t rollFileCnt = 10, //by tim , 0< rollFileCnt < kRollFileCntMax(1024), if = 0 then always roll 
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);

    //by zhuting(TIM)
  void checkRollFileCount(const char* fileName);

  static string getLogFileName(const string& basename, time_t* now);

  const string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;

  int count_;

  boost::scoped_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  boost::scoped_ptr<FileUtil::AppendFile> file_;

  const static int kRollPerSeconds_ = 60*60*24;

  //by TIM
  const static int kRollFileCntMax = 1024;
  size_t rollFileCnt_;
  std::deque<std::string> rollFileNames_;
};

}
#endif  // TIM_BASE_LOGFILE_H
