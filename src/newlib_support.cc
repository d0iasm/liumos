#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

extern "C" {

caddr_t sbrk(int incr) {
  errno = ENOMEM;
  return 0;
}
}