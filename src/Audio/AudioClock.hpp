//
//  AudioClock.hpp
//  dude_wake_up
//
//  The clock audio analysis and beat tracking read. It's the steady clock,
//  unless a test (an offline render) holds it: then it reads the held time,
//  so audio fed faster or slower than real time is analysed as if it played.
//  Header-only.
//

#ifndef AudioClock_hpp
#define AudioClock_hpp

#include <atomic>
#include <chrono>
#include <cstdint>

struct AudioClock {
  using time_point = std::chrono::steady_clock::time_point;

  static time_point now() {
    const int64_t held = heldUs().load(std::memory_order_relaxed);
    if (held < 0) return std::chrono::steady_clock::now();
    return time_point(std::chrono::microseconds(held));
  }

  // Held at `seconds` past the moment it was first held (< 0: the steady
  // clock again). Holding starts where the steady clock was, so nothing
  // timed before it sees time run backwards.
  static void hold(double seconds) {
    if (seconds < 0) {
      heldUs() = -1;
      baseUs() = -1;
      return;
    }
    if (baseUs() < 0)
      baseUs() = std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now().time_since_epoch()).count();
    heldUs() = baseUs() + static_cast<int64_t>(seconds * 1e6);
  }

  // A held clock moved on by `seconds` (it must be held).
  static void advance(double seconds) {
    heldUs() += static_cast<int64_t>(seconds * 1e6);
  }

private:
  static std::atomic<int64_t>& heldUs() { static std::atomic<int64_t> v{-1}; return v; }
  static std::atomic<int64_t>& baseUs() { static std::atomic<int64_t> v{-1}; return v; }
};

#endif /* AudioClock_hpp */
