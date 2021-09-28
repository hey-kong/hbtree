#pragma once

#include <thread>

class Daemon {
 private:
  virtual void worker() = 0;

 protected:
  std::thread thread_;

 public:
  Daemon() {}
  virtual ~Daemon() {}

  void start() {
    thread_ = std::thread(&Daemon::worker, this);
    thread_.detach();
  }
};
