#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "btree.h"
#include "nvm_common.h"

class Background {
 private:
  virtual void mainLoop() = 0;

 protected:
  thread thread_;
  mutex mu_;
  condition_variable cv_;

 public:
  Background() {}
  ~Background() {}

  void start() {
    thread_ = thread(&Background::mainLoop, this);
    thread_.detach();
  }
};

class LogBackground : public Background {
 private:
  char *apply_addr_;
  char *end_addr_;
  TOID(btree) bt_;

  virtual void mainLoop() {
    while (1) {
      unique_lock<mutex> lk(this->mu_);
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
  LogBackground(char *addr, TOID(btree) bt) {
    apply_addr_ = addr;
    end_addr_ = nullptr;
    bt_ = bt;
  }

  void update(char *addr) {
    lock_guard<mutex> lk(this->mu_);
    this->end_addr_ = addr;
    this->cv_.notify_one();
  }
};

class LogHandler {
 public:
  LogHandler(char *addr, TOID(btree) bt) {
    log_thread = new LogBackground(addr, bt);
    log_thread->start();
  }
  void update(char *addr) { log_thread->update(addr); }

 private:
  LogBackground *log_thread;
};
