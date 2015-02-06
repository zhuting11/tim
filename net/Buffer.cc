#include <tim/net/Buffer.h>
#include <tim/net/SocketsOps.h>

#include <errno.h>
//#include <sys/uio.h>

using namespace tim;
using namespace tim::net;

const char Buffer::kCRLF[] = "\r\n";

//const size_t Buffer::kCheapPrepend;
//const size_t Buffer::kInitialSize;



ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  //struct iovec vec[2];
  const size_t writable = writableBytes();
  //vec[0].iov_base = begin()+writerIndex_;
  //vec[0].iov_len = writable;
  //vec[1].iov_base = extrabuf;
  //vec[1].iov_len = sizeof extrabuf;

  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  //const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  //const ssize_t n = sockets::readv(fd, vec, iovcnt);
  const ssize_t n = sockets::read(fd, begin()+writerIndex_, writable);
  ssize_t nEx = 0;



  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) < writable)
  {
    writerIndex_ += n;
  }
  else
  {
	ssize_t nEx = sockets::read(fd, extrabuf, sizeof extrabuf);

	if(nEx < 0) 
		*savedErrno = errno;
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, nEx);
	}

  }

  return n+nEx;
}

