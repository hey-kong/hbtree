#include "nvm_allocator.h"

#include "inner_node.h"

void NVMLogFile::worker() {
  while (1) {
    if (IsFoot(apply_addr_)) {
      apply_addr_ = pmem_addr_;
    }
    while (memory_used_.load() == 0 && !finished) {
      // Wait for memory_used_ to update
    }
    if (memory_used_.load() == 0 && finished) {
      break;
    }

    LogNode *tmp = (LogNode *)malloc(LOGNODEBYTES);
    memcpy(tmp, apply_addr_, LOGNODEBYTES);
    if (tmp->type == logDeleteType) {
      D_RW(bt_)->btree_delete(tmp->key);
    } else if (tmp->type == logWriteType) {
      D_RW(bt_)->btree_insert(tmp->key, (char *)tmp->value);
    } else {
      int cnt = 0;
      entry_key_t keys[BTREEITEMS];
      unsigned long vals[BTREEITEMS];
      D_RW(bt_)->btree_search_range(tmp->key, tmp->value, keys, vals, cnt);
      // Here we get the new inner node address
      InnerNode *new_node = reinterpret_cast<InnerNode *>(tmp->type);
      {
        // Insert a batch of data into the new node
        unique_lock<mutex> lck(split_mut_);
        for (int i = 0; i < cnt; i++) {
          // Since only hot nodes split, writing logs is required in this call
          new_node->insert(keys[i], (char *)vals[i]);
        }
        split_ = true;
        split_cv_.notify_all();
      }
      for (int i = cnt - 1; i >= 0; i--) {
        D_RW(bt_)->btree_delete(keys[i]);
      }
    }
    apply_addr_ += LOGNODEBYTES;
    atomic_fetch_sub(&memory_used_, LOGNODEBITS);
    // printf("READ key(%lu)\n", tmp->key);
  }
  // TODO: If aborted, some KVs will not be applied to NVM.
}

NVMLogFile::NVMLogFile(const string path, size_t size, TOID(btree) bt) {
  size = size - (size % LOGNODEBITS) + LOGNODEBITS;
  pmem_addr_ = static_cast<char *>(pmem_map_file(
      path.c_str(), size, PMEM_FILE_CREATE, 0666, &mapped_len_, &is_pmem_));
  if (pmem_addr_ == NULL) {
    printf("%s: mapping failed, filepath %s, error: %s(%d)\n", __FUNCTION__,
           path.c_str(), strerror(errno), errno);
    exit(-1);
  } else {
    printf("%s: mapping at %p\n", __FUNCTION__, pmem_addr_);
  }

  apply_addr_ = pmem_addr_;
  cur_addr_ = pmem_addr_;
  capacity_ = size;
  memory_used_ = 0;
  bt_ = bt;
}

NVMLogFile::~NVMLogFile() { pmem_unmap(pmem_addr_, mapped_len_); }

char *NVMLogFile::GetPmemAddr() { return pmem_addr_; }

void NVMLogFile::Reset() {
  cur_addr_ = pmem_addr_;
  pmem_memset_persist(pmem_addr_, 0, mapped_len_);
}

bool NVMLogFile::IsFoot(char *addr) {
  return pmem_addr_ + (capacity_ / LOGNODEBITS * LOGNODEBYTES) == addr;
}

char *NVMLogFile::AllocateAligned() {
  mut_.lock();
  if (IsFoot(cur_addr_)) {
    cur_addr_ = pmem_addr_;
  }
  while (memory_used_.load() == capacity_) {
    // Wait for memory_used_ to update
  }

  char *result = cur_addr_;
  cur_addr_ += LOGNODEBYTES;
  mut_.unlock();
  return result;
}

char *NVMLogFile::Write(entry_key_t key, char *value) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = logWriteType;
  node.key = key;
  node.value = reinterpret_cast<uint64_t>(value);
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  atomic_fetch_add(&memory_used_, LOGNODEBITS);
  return log;
}

char *NVMLogFile::Delete(entry_key_t key) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = logDeleteType;
  node.key = key;
  node.value = 0;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  atomic_fetch_add(&memory_used_, LOGNODEBITS);
  return log;
}

char *NVMLogFile::Split(entry_key_t min, entry_key_t max, InnerNode *new_node) {
  char *log = this->AllocateAligned();
  LogNode node;
  node.type = reinterpret_cast<uint64_t>(new_node);
  node.key = min;
  node.value = max;
  timer.start();
  unique_lock<mutex> lck(split_mut_);
  split_ = false;
  memcpy(log, &node, LOGNODEBYTES);
  nvm_persist(log, LOGNODEBYTES);
  atomic_fetch_add(&memory_used_, LOGNODEBITS);
  split_cv_.wait(lck, [this] { return this->split_; });
  timer.printElapsed();
  return log;
}

void NVMLogFile::Recovery(NVMLogFile *log) {}
