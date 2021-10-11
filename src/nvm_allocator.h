#ifndef _NVM_ALLOCATOR_H_
#define _NVM_ALLOCATOR_H_

#include <err.h>
#include <errno.h>
#include <libpmem.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>

#include "btree.h"
#include "daemon.h"
#include "nvm_common.h"

using entry_key_t = int64_t;

inline void clflush(void *data, int len) { pmem_persist(data, len); }

class InnerNode;

class NVMLogFile : public Daemon {
 private:
  size_t mapped_len_;
  int is_pmem_;
  char *pmem_addr_;
  char *begin_addr_;
  char *apply_addr_;
  char *cur_addr_;
  uint64_t capacity_;
  atomic<uint64_t> memory_used_;
  mutex mut_;
  TOID(btree) bt_;

  void worker();

 public:
  NVMLogFile(const string path, size_t size, TOID(btree) bt);
  ~NVMLogFile();

  char *GetBeginAddr();
  char *GetPmemAddr();
  void Reset();
  bool IsFoot(char *);
  char *AllocateAligned();

  char *Write(entry_key_t, char *);
  char *Delete(entry_key_t);
  char *Split(entry_key_t, InnerNode *);
  void Recovery(NVMLogFile *);
};

#endif  // _NVM_ALLOCATOR_H_
