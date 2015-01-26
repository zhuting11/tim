#include <tim/base/Timestamp.h>

//#include <sys/time.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <tim/base/inttypes.h>
//#include <tim/base/stdint.h>
#undef __STDC_FORMAT_MACROS

#include <boost/static_assert.hpp>
#include <time.h>
#include <Windows.h>

using namespace tim;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

	GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;

    return (0);
}

//int gettimeofday(struct timeval * tp, struct timezone * tzp)
//{
//    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
//    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
//
//    SYSTEMTIME  system_time;
//    FILETIME    file_time;
//    uint64_t    time;
//
//    GetSystemTime( &system_time );
//    SystemTimeToFileTime( &system_time, &file_time );
//    time =  ((uint64_t)file_time.dwLowDateTime )      ;
//    time += ((uint64_t)file_time.dwHighDateTime) << 32;
//
//    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
//    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
//    return 0;
//}

Timestamp::Timestamp(int64_t microseconds)
  : microSecondsSinceEpoch_(microseconds)
{
}

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  _snprintf_s(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);

  return buf;
}

string Timestamp::toFormattedString(bool showMicroseconds) const
{
  char buf[32] = {0};
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  //gmtime_s(&tm_time, &seconds);

  localtime_s(&tm_time, &seconds);

  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    _snprintf_s(buf, sizeof(buf)-1, "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  }
  else
  {
    _snprintf_s(buf, sizeof(buf)-1, "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

  }
  return buf;
}

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid()
{
  return Timestamp();
}

