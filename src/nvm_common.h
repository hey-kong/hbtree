#pragma once

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

const uint64_t MemReserved = (1 << 10);
const uint64_t Hollow = (3 << 10);
condition_variable log_cv;

struct LogNode {
  uint64_t type : 1;
  uint64_t key : 63;
  uint64_t value;
};

enum LogType { logDeleteType = 0, logWriteType = 1 };
