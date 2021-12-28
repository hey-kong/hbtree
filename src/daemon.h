#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <atomic>
#include <thread>

class Daemon {
 private:
  virtual void worker() = 0;

 protected:
  atomic<bool> finished;
  std::thread thread_;

 public:
  Daemon() { finished = false; }
  virtual ~Daemon() { finished = true; }

  void start() {
    thread_ = std::thread(&Daemon::worker, this);
    thread_.detach();
  }
};

#endif  // _DAEMON_H_
