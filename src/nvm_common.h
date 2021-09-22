#pragma once

#include <libpmem.h>

static inline void nvm_persist(const void *addr, size_t len) {
  pmem_persist(addr, len);
}

static inline void nvm_memcpy_persist(void *pmemdest, const void *src,
                                      size_t len) {
  pmem_memcpy_persist(pmemdest, src, len);
}
