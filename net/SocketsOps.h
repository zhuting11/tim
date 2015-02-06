#ifndef TIM_NET_SOCKETSOPS_H
#define TIM_NET_SOCKETSOPS_H

//#include <arpa/inet.h>
#include <WinSock2.h>
#include <utility>

namespace tim
{
namespace net
{
namespace sockets
{

typedef   int   ssize_t;

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie();

int  connect(int sockfd, const struct sockaddr_in& addr);
void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int  accept(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toIpPort(char* buf, size_t size,
              const struct sockaddr_in& addr);
void toIp(char* buf, size_t size,
          const struct sockaddr_in& addr);
void fromIpPort(const char* ip, unsigned short port,
                  struct sockaddr_in* addr);

int getSocketError(int sockfd);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);
std::pair<SOCKET, SOCKET> creatSockPair();

}
}
}

#endif  // TIM_NET_SOCKETSOPS_H
