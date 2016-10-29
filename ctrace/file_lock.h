#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

namespace ctrace {

#define EINTR_LOOP(expr)                            \
        ({                                          \
            decltype(expr) ret;                   \
            do {                                    \
              ret = (expr);                         \
            } while (ret == -1 && errno == EINTR);  \
            ret;                                    \
         })

// A simple file lock wrapper offering lock/unlock interface to be
// compatible with std::mutex to use std::lock_guard.
class FileLock {
public:
  FileLock(int fd) : fd_(fd) {
    assert(fd_ > 0);
  }

  // Do nothing
  ~FileLock() {}

  void lock() {
    flock lock; 
    bzero(&lock, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    EINTR_LOOP(fcntl(fd_, F_SETLKW, &lock));
  }
  void unlock() {
    flock lock; 
    bzero(&lock, sizeof(lock));
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    fcntl(fd_, F_SETLK, &lock); 
  }
private:
  int fd_;
};

#undef EINTR_LOOP
}
