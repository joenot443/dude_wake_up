//
//  LifecycleCounter.hpp
//
//  Lightweight in-process leak-hunting tool. Drop a LifecycleCounter member
//  into a class and the registry tracks live instance counts by string name.
//
//  Usage:
//    class FileSource : public VideoSource {
//      ...
//      LifecycleCounter _lc_{"FileSource"};
//    };
//
//  Then `LifecycleRegistry::dumpJson()` returns a sorted snapshot.
//
//  Copy/move construction increments the new counter; destruction decrements.
//  So `live_count = ctors + copy_ctors + move_ctors - dtors`.
//

#ifndef LifecycleCounter_hpp
#define LifecycleCounter_hpp

#include <atomic>
#include <map>
#include <mutex>
#include <string>

namespace lifecycle {

class Registry {
public:
  static Registry& instance() {
    static Registry r;
    return r;
  }

  void increment(const char* name) {
    std::lock_guard<std::mutex> lock(mu_);
    counters_[name]++;
    constructed_[name]++;
  }

  void decrement(const char* name) {
    std::lock_guard<std::mutex> lock(mu_);
    counters_[name]--;
    destroyed_[name]++;
  }

  // Returns a JSON-shaped string with three sections: "live", "constructed",
  // "destroyed". `live` should hit zero after a clean add/remove cycle.
  std::string dumpJson() {
    std::lock_guard<std::mutex> lock(mu_);
    std::string s = "{\"live\":{";
    bool first = true;
    for (auto& [k, v] : counters_) {
      if (!first) s += ",";
      first = false;
      s += "\"" + std::string(k) + "\":" + std::to_string(v.load());
    }
    s += "},\"constructed\":{";
    first = true;
    for (auto& [k, v] : constructed_) {
      if (!first) s += ",";
      first = false;
      s += "\"" + std::string(k) + "\":" + std::to_string(v);
    }
    s += "},\"destroyed\":{";
    first = true;
    for (auto& [k, v] : destroyed_) {
      if (!first) s += ",";
      first = false;
      s += "\"" + std::string(k) + "\":" + std::to_string(v);
    }
    s += "}}";
    return s;
  }

  // Returns a flat human-readable line: "FileSource:3 Shader:1 Connection:0".
  // Hides zero-live entries unless `includeZero` is true.
  std::string dumpFlat(bool includeZero = false) {
    std::lock_guard<std::mutex> lock(mu_);
    std::string s;
    for (auto& [k, v] : counters_) {
      int live = v.load();
      if (!includeZero && live == 0) continue;
      if (!s.empty()) s += " ";
      s += std::string(k) + ":" + std::to_string(live);
    }
    return s;
  }

  // Reset all counters to zero (use to establish a baseline mid-run).
  void resetCounters() {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto& [k, v] : counters_) v = 0;
    constructed_.clear();
    destroyed_.clear();
  }

private:
  Registry() = default;
  std::mutex mu_;
  std::map<std::string, std::atomic<int>> counters_;
  std::map<std::string, int> constructed_;
  std::map<std::string, int> destroyed_;
};

// Member-by-value lifecycle counter. Constructing increments the registry,
// destroying decrements. Copying / moving each create a new live instance.
struct Counter {
  const char* name;
  explicit Counter(const char* n) : name(n) { Registry::instance().increment(n); }
  Counter(const Counter& o) : name(o.name) { Registry::instance().increment(name); }
  Counter(Counter&& o) noexcept : name(o.name) { Registry::instance().increment(name); }
  Counter& operator=(const Counter& o) {
    if (this != &o) {
      Registry::instance().decrement(name);
      name = o.name;
      Registry::instance().increment(name);
    }
    return *this;
  }
  Counter& operator=(Counter&& o) noexcept {
    if (this != &o) {
      Registry::instance().decrement(name);
      name = o.name;
      Registry::instance().increment(name);
    }
    return *this;
  }
  ~Counter() { Registry::instance().decrement(name); }
};

} // namespace lifecycle

// Drop into a class body to track live instances.
#define LIFECYCLE_TRACK(NameStr) ::lifecycle::Counter _lifecycleCounter_{NameStr}

#endif /* LifecycleCounter_hpp */
