#ifndef _NVM_COMMON_H_
#define _NVM_COMMON_H_

#include <libpmem.h>

static inline void nvm_persist(const void *addr, size_t len) {
  pmem_persist(addr, len);
}

static inline void nvm_memcpy_persist(void *pmemdest, const void *src,
                                      size_t len) {
  pmem_memcpy_persist(pmemdest, src, len);
}

#define LOGNODEBYTES 24
#define LOGNODEBITS (24 * 8)

struct LogNode {
  uint64_t type;
  uint64_t key;
  uint64_t value;
};

enum LogType { logDeleteType = 0, logWriteType = 1 };

#endif  // _NVM_COMMON_H_
