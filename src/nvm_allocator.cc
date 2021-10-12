#include "nvm_allocator.h"

#include "inner_node.h"

void NVMLogFile::worker() {
  while (1) {
    if (IsFoot(apply_addr_)) {
      apply_addr_ = pmem_addr_ + Hollow;
    }
    while (memory_used_.load() == 0) {
      // Wait for memory_used_ to update
    }

    LogNode *tmp = (LogNode *)malloc(LOGNODEBYTES);
    memcpy(tmp, apply_addr_, LOGNODEBYTES);
    if (tmp->type == logDeleteType) {
      D_RW(bt_)->btree_delete(tmp->key);
    } else if (tmp->type == logWriteType) {
      D_RW(bt_)->btree_insert(tmp->key, (char *)tmp->value);
    } else if (tmp->type == logSplitType) {
      entry_key_t keys[BTREEITEMS];
      unsigned long vals[BTREEITEMS];
      int cnt;
      D_RW(bt_)->btree_search_to_end(tmp->key, keys, vals, cnt);
      InnerNode *new_node = (InnerNode *)tmp->value;
      for (int i = 0; i < cnt; i++) {
        new_node->insert(keys[i], (char *)vals[i]);
      }
      // TODO: Notification required.
      for (int i = 0; i < cnt; i++) {
        D_RW(bt_)->btree_delete(keys[i]);
      }
    }
    apply_addr_ += LOGNODEBITS;
    atomic_fetch_sub(&memory_used_, LOGNODEBITS);
    // printf("READ key(%lu)\n", tmp->key);
  }
  // TODO: If aborted, some KVs will not be applied to NVM.
}

NVMLogFile::NVMLogFile(const string path, size_t size, TOID(btree) bt) {
  assert(size > Hollow);
  size = size - ((size - Hollow) % LOGNODEBITS) + LOGNODEBITS;
  pmem_addr_ = static_cast<char *>(pmem_map_file(
      path.c_str(), size, PMEM_FILE_CREATE, 0666, &mapped_len_, &is_pmem_));
  if (pmem_addr_ == NULL) {
    printf("%s: mapping failed, filepath %s, error: %s(%d)\n", __FUNCTION__,
           path.c_str(), strerror(errno), errno);
    exit(-1);
  } else {
    printf("%s: mapping at %p\n", __FUNCTION__, pmem_addr_);
  }

  begin_addr_ = pmem_addr_ + Hollow;
  apply_addr_ = begin_addr_;
  cur_addr_ = begin_addr_;
  capacity_ = size;
  memory_used_ = 0;
  bt_ = bt;
  pmem_memset_persist(pmem_addr_, 0, Hollow);
}

NVMLogFile::~NVMLogFile() { pmem_unmap(pmem_addr_, mapped_len_); }

char *NVMLogFile::GetBeginAddr() { return begin_addr_; }

char *NVMLogFile::GetPmemAddr() { return pmem_addr_; }

void NVMLogFile::Reset() {
  cur_addr_ = pmem_addr_;
  pmem_memset_persist(pmem_addr_, 0, mapped_len_);
}

bool NVMLogFile::IsFoot(char *addr) {
  return (uint64_t)(addr - pmem_addr_) == capacity_;
}

char *NVMLogFile::AllocateAligned() {
  mut_.lock();
  if (IsFoot(cur_addr_)) {
    cur_addr_ = pmem_addr_ + Hollow;
  }
  while (memory_used_.load() == capacity_ - Hollow) {
    // Wait for memory_used_ to update
  }

  char *result = cur_addr_;
  cur_addr_ += LOGNODEBITS;
  atomic_fetch_add(&memory_used_, LOGNODEBITS);
  mut_.unlock();
  return result;
}

char *NVMLogFile::Write(entry_key_t key, char *value) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = logWriteType;
  node.key = key;
  node.value = (uint64_t)value;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  return this->cur_addr_;
}

char *NVMLogFile::Delete(entry_key_t key) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = logDeleteType;
  node.key = key;
  node.value = 0;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  return this->cur_addr_;
}

char *NVMLogFile::Split(entry_key_t right_boundary, InnerNode *new_node) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = logSplitType;
  node.key = right_boundary;
  node.value = (uint64_t)new_node;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  return this->cur_addr_;
}

void NVMLogFile::Recovery(NVMLogFile *log) {}