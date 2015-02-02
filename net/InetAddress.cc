#include <tim/net/InetAddress.h>

#include <tim/base/Logging.h>
#include <tim/net/Endian.h>
#include <tim/net/SocketsOps.h>

//#include <netdb.h>
//#include <strings.h>  // bzero
//#include <netinet/in.h>

#include <boost/static_assert.hpp>

#ifdef WIN32
#define bzero(a,b) memset(a, 0, b)
#endif // WIN32


//// INADDR_ANY use (type)value casting.
//#pragma GCC diagnostic ignored "-Wold-style-cast"


typedef uint32_t in_addr_t;
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
//#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

using namespace tim;
using namespace tim::net;

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
  //bzero(&addr_, sizeof addr_);
  memset(&addr_, 0, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
  addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
  addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(StringArg ip, uint16_t port)
{
  bzero(&addr_, sizeof addr_);
  sockets::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIpPort() const
{
  char buf[32];
  sockets::toIpPort(buf, sizeof buf, addr_);
  return buf;
}

string InetAddress::toIp() const
{
  char buf[32];
  sockets::toIp(buf, sizeof buf, addr_);
  return buf;
}

static __declspec(thread) char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
  assert(out != NULL);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  //int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
  he = gethostbyname(hostname.c_str());
  //if (ret == 0 && he != NULL)
  if (he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else
  {
    //if (ret)
    //{
    LOG_SYSERR << "InetAddress::resolve";
    //}
    return false;
  }
}
