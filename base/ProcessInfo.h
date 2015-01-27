#ifndef TIM_BASE_PROCESSINFO_H
#define TIM_BASE_PROCESSINFO_H

#include <tim/base/StringPiece.h>
#include <tim/base/Types.h>
#include <tim/base/Timestamp.h>
#include <vector>
#include <tim/base/Mutex.h>

namespace tim
{

namespace ProcessInfo
{

	typedef int uid_t;

  pid_t pid();
  string pidString();
  //uid_t uid();
  //string username();
  //uid_t euid();
  Timestamp startTime();
  int clockTicksPerSecond();
  int pageSize();
  bool isDebugBuild();  // constexpr

  string hostname();
  string procname();
  StringPiece procname(const string& stat);

 /* /// read /proc/self/status
  string procStatus();

  /// read /proc/self/stat
  string procStat();

  /// read /proc/self/task/tid/stat
  string threadStat();

  /// readlink /proc/self/exe
  string exePath();

  int openedFiles();
  int maxOpenFiles();

  struct CpuTime
  {
    double userSeconds;
    double systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }
  };
  CpuTime cpuTime();

  int numThreads();
  std::vector<pid_t> threads();*/
}

}

#endif  // TIM_BASE_PROCESSINFO_H
