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

#define LOGPATH "/pmem0/log_persistent"
#define LOGSIZE 64 * (1 << 20)

using entry_key_t = int64_t;

enum LogType { logDeleteType = 0, logWriteType = 1 };

const uint64_t mem_reserved = (1 << 10);  // 保留 1K 空间

inline void clflush(void *data, int len) { pmem_persist(data, len); }

/*
class LogWriter;

class NVMAllocator {
 private:
  char *pmem_addr_;
  size_t mapped_len_;
  uint64_t capacity_;
  uint64_t memory_used_;
  int is_pmem_;
  std::mutex mut_;
  char *cur_index_;

 public:
  NVMAllocator(const std::string path, size_t size) {
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

    capacity_ = size;
    memory_used_ = 0;
    cur_index_ = pmem_addr_;
  }

  ~NVMAllocator() { pmem_unmap(pmem_addr_, mapped_len_); }

  char *Allocate(size_t bytes, uint64_t aligns = 64) {
    std::lock_guard<std::mutex> lk(mut_);
    uint64_t start_offset = (uint64_t)cur_index_;
    uint64_t allocated =
        (bytes + aligns - 1) & (~(aligns - 1));  // 保证最小按照aligns分配
    start_offset =
        (start_offset + aligns - 1) & (~(aligns - 1));  // 按照aligns对齐
    memory_used_ = start_offset + allocated - (uint64_t)pmem_addr_;
    cur_index_ = (char *)(start_offset + allocated);
    return (char *)start_offset;
  }

  char *AllocateAligned(size_t bytes, size_t huge_page_size = 0) {
    mut_.lock();
    char *result = cur_index_;
    cur_index_ += bytes;
    memory_used_ += bytes;
    mut_.unlock();
    return result;
  }

  LogWriter *getNVMptr(uint64_t off) {
    if (off == -1) return nullptr;
    return (LogWriter *)(pmem_addr_ + off);
  }

  uint64_t getOff(char *ptr) { return (uint64_t)ptr - (uint64_t)pmem_addr_; }

  void ResetZero() { pmem_memset_persist(pmem_addr_, 0, mapped_len_); }

  void PrintStorage(void) {
    printf("Storage capacity is %lluG %lluM %lluK %lluB\n", capacity_ >> 30,
           capacity_ >> 20 & (1024 - 1), capacity_ >> 10 & (1024 - 1),
           capacity_ & (1024 - 1));
    printf("Storage used is %lluG %lluM %lluK %lluB\n", memory_used_ >> 30,
           memory_used_ >> 20 & (1024 - 1), memory_used_ >> 10 & (1024 - 1),
           memory_used_ & (1024 - 1));
  }

  size_t BlockSize() { return 0; }

  bool StorageIsFull() { return memory_used_ + mem_reserved >= capacity_; }
};
*/

class LogNode {
 private:
  uint64_t type_ : 1;
  uint64_t key_ : 63;
  uint64_t value_;

 public:
  LogNode(uint64_t type, uint64_t key, uint64_t value)
      : type_(type), key_(key), value_(value){};
};

#define LOGNODEBYTES 16

class NVMLogFile {
 private:
  size_t mapped_len_;
  int is_pmem_;
  char *pmem_addr_;
  char *begin_addr_;
  char *cur_index_;
  uint64_t capacity_;
  uint64_t memory_used_;
  std::mutex mut_;

 public:
  NVMLogFile(const std::string path, size_t size) {
    //映射NVM空间到文件
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
    cur_index_ = begin_addr_;
    capacity_ = size;
    memory_used_ = 0;
    pmem_memset_persist(pmem_addr_, 0, 4096);
  }

  ~NVMLogFile() { pmem_unmap(pmem_addr_, mapped_len_); }

  char *GetBeginAddr() { return begin_addr_; }

  char *GetPmemAddr(uint64_t off) { return begin_addr_ + off; }

  void Reset() {
    cur_index_ = pmem_addr_;
    pmem_memset_persist(pmem_addr_, 0, mapped_len_);
  }

  bool Expand(uint64_t size) { memory_used_ += size; }

  bool IsFull() { return memory_used_ + mem_reserved >= capacity_; }

  char *AllocateAligned(size_t bytes, size_t huge_page_size = 0) {
    mut_.lock();
    char *result = cur_index_;
    cur_index_ += bytes;
    memory_used_ += bytes;
    mut_.unlock();
    return result;
  }

  void Write(entry_key_t key, char *value);
  void Delete(entry_key_t key);
  void Recovery(NVMLogFile *log);
};

/*
 *  class NVMLogFile
 */
void NVMLogFile::Write(entry_key_t key, char *value) {
  char *logvalue = this->AllocateAligned(LOGNODEBYTES);
  LogNode node(logWriteType, key, (uint64_t)value);
  memcpy(logvalue, &node, LOGNODEBYTES);
  nvm_persist(logvalue, LOGNODEBYTES);
}

void NVMLogFile::Delete(entry_key_t key) {
  char *logvalue = this->AllocateAligned(LOGNODEBYTES);
  LogNode node(logDeleteType, key, 0);
  memcpy(logvalue, &node, LOGNODEBYTES);
  nvm_persist(logvalue, LOGNODEBYTES);
}
