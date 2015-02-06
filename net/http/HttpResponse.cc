#include <tim/net/http/HttpResponse.h>
#include <tim/net/Buffer.h>

#include <stdio.h>

using namespace tim;
using namespace tim::net;

void HttpResponse::appendToBuffer(Buffer* output) const
{
  char buf[32];
  _snprintf_s(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
  output->append(buf);
  output->append(statusMessage_);
  output->append("\r\n");

  if (closeConnection_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    _snprintf_s(buf, sizeof buf, sizeof buf, "Content-Length: %d\r\n", body_.size());
    output->append(buf);
    output->append("Connection: Keep-Alive\r\n");
  }

  for (std::map<string, string>::const_iterator it = headers_.begin();
       it != headers_.end();
       ++it)
  {
    output->append(it->first);
    output->append(": ");
    output->append(it->second);
    output->append("\r\n");
  }

  output->append("\r\n");
  output->append(body_);
}
