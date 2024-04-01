#pragma once

#include <chrono>
#include <functional>
#include <map>

namespace mango {
typedef uint32_t TimerHandle;

struct Timer {
  float interval;
  std::function<void(void)> func;
  bool loop;

  float current_time;
};

class TimerManager {
public:
  void init();
  void tick(float delta_time);
  void destroy();

  TimerHandle addTimer(float interval, const std::function<void(void)> &func,
                       bool loop = false, bool loop_im_call = false);
  void removeTimer(TimerHandle timer_handle);

  float getTime() { return m_time; }

private:
  uint32_t m_timer_handle;
  std::map<TimerHandle, Timer> m_timers;

  float m_time;
};

class StopWatch {
public:
  void start();
  long long stopMs();
  float stop();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_end_time;
};

class TimeOuter {
public:
  TimeOuter(float out_time);

  void trigger();
  bool isTimeOut() const;

private:
  float out_time;
  long long trigger_timestamp;
};

long long getTimestampMs();
} // namespace mango