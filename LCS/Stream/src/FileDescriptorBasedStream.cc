#include <lofar_config.h>

#include <Stream/FileDescriptorBasedStream.h>
#include <Stream/SystemCallException.h>

#include <unistd.h>

#include <stdexcept>


namespace LOFAR {

FileDescriptorBasedStream::~FileDescriptorBasedStream()
{
  if (close(fd) < 0)
    throw SystemCallException("close");
}


void FileDescriptorBasedStream::read(void *ptr, size_t size)
{
  while (size > 0) {
    ssize_t bytes = ::read(fd, ptr, size);
    
    if (bytes < 0)
      throw SystemCallException("read");

    if (bytes == 0) 
      throw std::logic_error("read: premature end of file");

    size -= bytes;
    ptr   = static_cast<char *>(ptr) + bytes;
  }
}


void FileDescriptorBasedStream::write(const void *ptr, size_t size)
{
  while (size > 0) {
    ssize_t bytes = ::write(fd, ptr, size);

    if (bytes < 0)
      throw SystemCallException("write");

    size -= bytes;
    ptr   = static_cast<const char *>(ptr) + bytes;
  }
}

} // namespace LOFAR
