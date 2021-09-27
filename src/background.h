#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "btree.h"
#include "nvm_common.h"

class Background {
 private:
  virtual void worker() = 0;

 protected:
  thread thread_;
  mutex mu_;
  condition_variable cv_;

 public:
  Background() {}
  ~Background() {}

  void start() {
    thread_ = thread(&Background::worker, this);
    thread_.detach();
  }
};

class LogBackground : public Background {
 private:
  char *pmem_addr_;
  char *apply_addr_;
  char *end_addr_;
  uint64_t capacity_;
  TOID(btree) bt_;
  uint64_t loop_;

  virtual void worker() {
    memcpy(pmem_addr_, &loop_, 8);
    while (1) {
      unique_lock<mutex> lk(this->mu_);
      if (IsFull()) {
        loop_++;
        memcpy(pmem_addr_, &loop_, 8);
        apply_addr_ = pmem_addr_ + Hollow;
        log_cv.notify_one();
      }
      while (this->apply_addr_ == this->end_addr_ ||
             nullptr == this->end_addr_) {
        this->cv_.wait(lk);
        cout << "SLEEP" << endl;
      }

      LogNode tmp;
      memcpy(&tmp, apply_addr_, LOGNODEBYTES);
      if (tmp.type == logDeleteType) {
        D_RW(bt_)->btree_delete(tmp.key);
      } else if (tmp.type == logWriteType) {
        D_RW(bt_)->btree_insert(tmp.key, (char *)tmp.value);
      }
      cout << "READ " << tmp.key << endl;
      apply_addr_ += LOGNODEBYTES;
    }
    // TODO: If aborted, some KVs will not be applied to NVM.
  }

 public:
  LogBackground(char *pmem_addr, char *apply_addr, uint64_t capacity,
                TOID(btree) bt) {
    pmem_addr_ = pmem_addr;
    apply_addr_ = apply_addr;
    end_addr_ = nullptr;
    capacity_ = capacity;
    bt_ = bt;
    loop_ = 1;
  }

  bool IsFull() { return apply_addr_ - pmem_addr_ + MemReserved >= capacity_; }

  void update(char *addr) {
    lock_guard<mutex> lk(this->mu_);
    this->end_addr_ = addr;
    this->cv_.notify_one();
  }
};

class LogHandler {
 public:
  LogHandler(char *pmem_addr, char *apply_addr, uint64_t capacity,
             TOID(btree) bt) {
    log_thread = new LogBackground(pmem_addr, apply_addr, capacity, bt);
    log_thread->start();
  }
  void update(char *addr) { log_thread->update(addr); }

 private:
  LogBackground *log_thread;
};
