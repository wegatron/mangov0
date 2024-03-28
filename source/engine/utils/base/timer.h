#pragma once
#include <chrono>
namespace mango {
class StopWatch {
public:
  void start() { start_time_ = std::chrono::high_resolution_clock::now(); }
  uint64_t stopMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now() - start_time_)
        .count();
  }

  float stop() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now() - start_time_)
               .count() *
           1e-9f;
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};
} // namespace mango