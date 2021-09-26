#pragma once

#include <err.h>
#include <errno.h>
#include <libpmem.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>

#include "nvm_common.h"

using entry_key_t = int64_t;

const uint64_t MemReserved = (1 << 10);

inline void clflush(void *data, int len) { pmem_persist(data, len); }

class NVMLogFile {
 private:
  size_t mapped_len_;
  int is_pmem_;
  char *pmem_addr_;
  char *begin_addr_;
  char *cur_addr_;
  uint64_t capacity_;
  uint64_t memory_used_;
  std::mutex mut_;

 public:
  NVMLogFile(const std::string path, size_t size) {
    // 映射NVM空间到文件
    pmem_addr_ = static_cast<char *>(pmem_map_file(
        path.c_str(), size, PMEM_FILE_CREATE, 0666, &mapped_len_, &is_pmem_));
    if (pmem_addr_ == NULL) {
      printf("%s: mapping failed, filepath %s, error: %s(%d)\n", __FUNCTION__,
             path.c_str(), strerror(errno), errno);
      exit(-1);
    } else {
      printf("%s: mapping at %p\n", __FUNCTION__, pmem_addr_);
    }

    begin_addr_ = pmem_addr_ + 4096;
    cur_addr_ = begin_addr_;
    capacity_ = size;
    memory_used_ = 0;
    pmem_memset_persist(pmem_addr_, 0, 4096);
  }

  ~NVMLogFile() { pmem_unmap(pmem_addr_, mapped_len_); }

  char *GetBeginAddr() { return begin_addr_; }

  char *GetPmemAddr(uint64_t off) { return begin_addr_ + off; }

  void Reset() {
    cur_addr_ = pmem_addr_;
    pmem_memset_persist(pmem_addr_, 0, mapped_len_);
  }

  bool IsFull() { return memory_used_ + MemReserved >= capacity_; }

  char *AllocateAligned(size_t bytes, size_t huge_page_size = 0) {
    mut_.lock();
    char *result = cur_addr_;
    cur_addr_ += bytes;
    memory_used_ += bytes;
    mut_.unlock();
    return result;
  }

  char *Write(entry_key_t key, char *value);
  char *Delete(entry_key_t key);
  void Recovery(NVMLogFile *log);
};

/*
 *  class NVMLogFile
 */
char *NVMLogFile::Write(entry_key_t key, char *value) {
  char *log = this->AllocateAligned(LOGNODEBYTES);
  LogNode node;
  node.type = logWriteType;
  node.key = key;
  node.value = (uint64_t)value;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  return this->cur_addr_;
}

char *NVMLogFile::Delete(entry_key_t key) {
  char *log = this->AllocateAligned(LOGNODEBYTES);
  LogNode node;
  node.type = logDeleteType;
  node.key = key;
  node.value = 0;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  return this->cur_addr_;
}
