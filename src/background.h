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
  std::thread thread_;
  std::mutex mu_;
  std::condition_variable cv_;

 public:
  Background() {}
  ~Background() {}

  void start() {
    thread_ = std::thread(&Background::mainLoop, this);
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
      std::unique_lock<std::mutex> lk(this->mu_);
      while (this->apply_addr_ == this->end_addr_ ||
             nullptr == this->end_addr_) {
        this->cv_.wait(lk);
        std::cout << "SLEEP" << std::endl;
      }

      LogNode tmp;
      memcpy(&tmp, apply_addr_, LOGNODEBYTES);
      if (tmp.type == logDeleteType) {
        D_RW(bt_)->btree_delete(tmp.key);
      } else if (tmp.type == logWriteType) {
        D_RW(bt_)->btree_insert(tmp.key, (char *)tmp.value);
      }
      std::cout << "READ " << tmp.key << std::endl;
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
    std::lock_guard<std::mutex> lk(this->mu_);
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
