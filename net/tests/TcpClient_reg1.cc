// TcpClient::stop() called in the same iteration of IO event

#include <tim/base/Logging.h>
#include <tim/net/EventLoop.h>
#include <tim/net/TcpClient.h>

#include <boost/bind.hpp>

using namespace tim;
using namespace tim::net;

TcpClient* g_client;

void timeout()
{
  LOG_INFO << "timeout";
  g_client->stop();
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 2); // no such server
  TcpClient client(&loop, serverAddr, "TcpClient");
  g_client = &client;
  loop.runAfter(0.0, timeout);
  loop.runAfter(1.0, boost::bind(&EventLoop::quit, &loop));
  client.connect();
  CurrentThread::sleepUsec(100 * 1000);
  loop.loop();
}
