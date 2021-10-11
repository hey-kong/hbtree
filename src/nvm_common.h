#ifndef _NVM_COMMON_H_
#define _NVM_COMMON_H_

#include <libpmem.h>

#include <condition_variable>

static inline void nvm_persist(const void *addr, size_t len) {
  pmem_persist(addr, len);
}

static inline void nvm_memcpy_persist(void *pmemdest, const void *src,
                                      size_t len) {
  pmem_memcpy_persist(pmemdest, src, len);
}

#define LOGNODEBYTES 16
#define LOGNODEBITS (16 * 8)

const uint64_t Hollow = (4 << 10);

struct LogNode {
  uint64_t type : 2;
  uint64_t key : 62;
  uint64_t value;
};

enum LogType { logDeleteType = 0, logWriteType = 1, logSplitType = 2 };

#endif  // _NVM_COMMON_H_
