#ifndef TIM_BASE_EXCEPTION_H
#define TIM_BASE_EXCEPTION_H

#include <tim/base/Types.h>
#include <exception>

namespace tim
{

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  void fillStackTrace();

  string message_;
  string stack_;
};

}

#endif  // TIM_BASE_EXCEPTION_H
