#ifndef TIM_BASE_TIMEZONE_H
#define TIM_BASE_TIMEZONE_H

#include <tim/base/copyable.h>
#include <boost/shared_ptr.hpp>
#include <time.h>

namespace tim
{

// TimeZone for 1970~2030
class TimeZone //: public tim::copyable
{
 public:
  explicit TimeZone(const char* zonefile);
  TimeZone(int eastOfUtc, const char* tzname);  // a fixed timezone
  TimeZone() {}  // an invalid timezone

  // default copy ctor/assignment/dtor are Okay.

  bool valid() const
  {
    // 'explicit operator bool() const' in C++11
    return static_cast<bool>(data_);
  }

  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;

  // gmtime(3)
  static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
  // timegm(3)
  static time_t fromUtcTime(const struct tm&);
  // year in [1900..2500], month in [1..12], day in [1..31]
  static time_t fromUtcTime(int year, int month, int day,
                            int hour, int minute, int seconds);

  struct Data;

 private:

  boost::shared_ptr<Data> data_;
};

}
#endif  // TIM_BASE_TIMEZONE_H
