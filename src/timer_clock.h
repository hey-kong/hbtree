#ifndef _TIMER_CLOCK_H_
#define _TIMER_CLOCK_H_

#include <chrono>
#include <iostream>
#include <type_traits>

namespace Time {

using ns = std::chrono::nanoseconds;
using us = std::chrono::microseconds;
using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;
using m = std::chrono::minutes;
using h = std::chrono::hours;

template <typename DurationType>
class TimerClock;
using TimerClockNs = TimerClock<ns>;
using TimerClockUs = TimerClock<us>;
using TimerClockMs = TimerClock<ms>;
using TimerClockS = TimerClock<s>;
using TimerClockM = TimerClock<m>;
using TimerClockH = TimerClock<h>;

template <typename DurationType = ms>
class TimerClock {
 private:
  std::chrono::steady_clock::time_point start_;

 public:
  void start() { start_ = std::chrono::steady_clock::now(); }

  typename DurationType::rep elapsed() const {
    auto elapsed = std::chrono::duration_cast<DurationType>(
        std::chrono::steady_clock::now() - start_);
    return elapsed.count();
  }

  void printElapsed() {
    auto time = elapsed();

    if (std::is_same<DurationType, ns>::value) {
      std::cout << "Elapsed: " << time << " ns" << std::endl;
    } else if (std::is_same<DurationType, us>::value) {
      std::cout << "Elapsed: " << time << " us" << std::endl;
    } else if (std::is_same<DurationType, ms>::value) {
      std::cout << "Elapsed: " << time << " ms" << std::endl;
    } else if (std::is_same<DurationType, s>::value) {
      std::cout << "Elapsed: " << time << " s" << std::endl;
    } else if (std::is_same<DurationType, m>::value) {
      std::cout << "Elapsed: " << time << " m" << std::endl;
    } else if (std::is_same<DurationType, h>::value) {
      std::cout << "Elapsed: " << time << " h" << std::endl;
    }
  }
};

}  // namespace Time

#endif  // _TIMER_CLOCK_H_
