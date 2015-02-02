#include <tim/net/SocketsOps.h>

#include <tim/base/Logging.h>
#include <tim/base/Types.h>
#include <tim/net/Endian.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
//#include <strings.h>  // bzero
//#include <sys/socket.h>
//#include <unistd.h>
//#include <WinSock2.h>
#include <WS2tcpip.h>
//#include <Ws2def.h>

using namespace tim;
using namespace tim::net;

namespace
{

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
  return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
  return static_cast<SA*>(implicit_cast<void*>(addr));
}

//#if VALGRIND
void setNonBlockAndCloseOnExec(int sockfd)
{
	u_long ul = 1;
	ioctlsocket(sockfd, FIONBIO, &ul);     
  //// non-block
  //int flags = ::fcntl(sockfd, F_GETFL, 0);
  //flags |= O_NONBLOCK;
  //int ret = ::fcntl(sockfd, F_SETFL, flags);
  //// FIXME check

  //// close-on-exec
  //flags = ::fcntl(sockfd, F_GETFD, 0);
  //flags |= FD_CLOEXEC;
  //ret = ::fcntl(sockfd, F_SETFD, flags);
  //// FIXME check

  //(void)ret;
}
//#endif

}

int sockets::createNonblockingOrDie()
{
//#if VALGRIND
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0)
  {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }

  setNonBlockAndCloseOnExec(sockfd);
//#else
//  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
//  if (sockfd < 0)
//  {
//    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
//  }
//#endif
  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
  int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
  if (ret < 0)
  {
    LOG_SYSFATAL << "sockets::bindOrDie";
  }
}

void sockets::listenOrDie(int sockfd)
{
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0)
  {
    LOG_SYSFATAL << "sockets::listenOrDie";
  }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
//#if VALGRIND
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
//#else
//  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
//                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
//#endif
  if (connfd < 0)
  {
    int savedErrno = errno;
    LOG_SYSERR << "Socket::accept";
    switch (savedErrno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr_in& addr)
{
  return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
}

using namespace tim::net::sockets;

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
  //if (::close(sockfd) < 0)
  if (::closesocket(sockfd) < 0)
  {
    LOG_SYSERR << "sockets::close";
  }
}

void sockets::shutdownWrite(int sockfd)
{
  //if (::shutdown(sockfd, SHUT_WR) < 0)
  if (::shutdown(sockfd, SD_SEND) < 0)
  {
    LOG_SYSERR << "sockets::shutdownWrite";
  }
}

void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr_in& addr)
{
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, (PVOID)&addr.sin_addr, buf, static_cast<socklen_t>(size));
  size_t end = ::strlen(buf);
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  assert(size > end);
  _snprintf_s(buf+end, size-end, size-end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size,
                   const struct sockaddr_in& addr)
{
  assert(size >= INET_ADDRSTRLEN);
  ::inet_ntop(AF_INET, (PVOID)&addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void sockets::fromIpPort(const char* ip, unsigned short port, struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

int sockets::getSocketError(int sockfd)
{
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
  struct sockaddr_in localaddr;
  //bzero(&localaddr, sizeof localaddr);
  memset(&localaddr, 0, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
  struct sockaddr_in peeraddr;
  //bzero(&peeraddr, sizeof peeraddr);
  memset(&peeraddr, 0, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getPeerAddr";
  }
  return peeraddr;
}

bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

