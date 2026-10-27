//
//  TapTempo.hpp
//  dude_wake_up
//
//  Tempo and beat position from taps.
//
//  The recent taps (up to 16) are numbered as beats, allowing for missed
//  taps (an interval of about two beats is two beats), and a straight line
//  is fitted through them: its slope is the beat period, and its value at
//  the last tap is where that beat was, with human timing wobble averaged
//  out. A tap far off the line is dropped; an accidental double tap is
//  ignored; a deliberate change of tempo (taps that keep landing off the
//  line the same way) starts the fit again from the new taps. A pause of a
//  few beats starts over.
//
//  Header-only. Times are seconds on any steady clock.
//

#ifndef TapTempo_hpp
#define TapTempo_hpp

#include <algorithm>
#include <cmath>
#include <vector>

class TapTempo {
public:
  struct Result {
    bool valid = false;    // two taps or more
    double bpm = 0.0;
    double beatTime = 0.0; // where the latest tapped beat falls, on the fitted line
    int taps = 0;          // taps the estimate is from
  };

  static constexpr int kMaxTaps = 16;

  // A tap at `time`. Returns the estimate so far.
  Result tap(double time) {
    if (!taps_.empty()) {
      const double since = time - taps_.back();
      if (since < 0 || since > resetAfter()) taps_.clear();
      // A second tap much sooner than a beat: a double tap, not a beat.
      else if (result_.valid ? since < 0.35 * period() : since < 0.2) return result_;
    }
    taps_.push_back(time);
    if (taps_.size() > size_t(kMaxTaps)) taps_.erase(taps_.begin());
    fit();
    // Taps drifting off the line: a new tempo, from the taps since.
    if (result_.valid && changedTempo()) {
      taps_.erase(taps_.begin(), taps_.end() - 4);
      fit();
    }
    return result_;
  }

  void reset() {
    taps_.clear();
    result_ = Result{};
  }

  const Result& result() const { return result_; }

private:
  double period() const { return result_.valid ? 60.0 / result_.bpm : 0.5; }

  // A pause of 2.5 beats (at least 2 s) starts over.
  double resetAfter() const { return std::max(2.0, 2.5 * period()); }

  // Each tap's beat number for `period`, interval by interval (a missed
  // tap's interval is two beats). Numbering from the first tap instead
  // let a rough first period put later taps whole beats out.
  std::vector<double> beatNumbers(double period) const {
    std::vector<double> n(taps_.size(), 0.0);
    for (size_t i = 1; i < taps_.size(); ++i)
      n[i] = n[i - 1] + std::max(1.0, std::round((taps_[i] - taps_[i - 1]) / period));
    return n;
  }

  // Least squares t = a + b n over the taps (skipping `skip`, if any).
  static bool line(const std::vector<double>& t, const std::vector<double>& n, long skip, double& a, double& b) {
    double sn = 0, st = 0, snn = 0, snt = 0;
    int count = 0;
    for (size_t i = 0; i < t.size(); ++i) {
      if (long(i) == skip) continue;
      sn += n[i]; st += t[i]; snn += n[i] * n[i]; snt += n[i] * t[i];
      ++count;
    }
    const double d = count * snn - sn * sn;
    if (count < 2 || d <= 0) return false;
    b = (count * snt - sn * st) / d;
    a = (st - b * sn) / count;
    return b > 0;
  }

  void fit() {
    result_.valid = false;
    result_.taps = int(taps_.size());
    if (taps_.size() < 2) return;
    // A first period: the median interval (so a missed tap doesn't count).
    std::vector<double> intervals;
    for (size_t i = 1; i < taps_.size(); ++i) intervals.push_back(taps_[i] - taps_[i - 1]);
    std::nth_element(intervals.begin(), intervals.begin() + long(intervals.size() / 2), intervals.end());
    double b = intervals[intervals.size() / 2], a = taps_[0];
    std::vector<double> n;
    // Number the beats, fit, and again with the fitted period.
    for (int pass = 0; pass < 2; ++pass) {
      n = beatNumbers(b);
      if (!line(taps_, n, -1, a, b)) return;
    }
    // The worst tap, if it's far off the line (over a fifth of a beat), is a
    // slip: fit without it.
    long worst = -1;
    double worstError = 0;
    for (size_t i = 0; i < taps_.size(); ++i) {
      const double e = std::fabs(taps_[i] - (a + b * n[i]));
      if (e > worstError) { worstError = e; worst = long(i); }
    }
    if (taps_.size() >= 4 && worstError > 0.2 * b) {
      double a2, b2;
      if (line(taps_, n, worst, a2, b2)) { a = a2; b = b2; }
    }
    a_ = a; b_ = b; n_ = n;
    result_.valid = true;
    result_.bpm = 60.0 / b;
    result_.beatTime = a + b * n.back();
  }

  // The last four taps against a line through the taps before them: all
  // late (or all early), and the newest further off than the line's own
  // uncertainty allows (4 standard errors of its prediction there, from
  // how much the earlier taps wobble, and at least 12% of a beat). Timing
  // wobble and an imprecise early line don't do that; a new tempo does.
  bool changedTempo() const {
    if (taps_.size() < 8) return false;
    const size_t m = taps_.size() - 4;
    const std::vector<double> t(taps_.begin(), taps_.begin() + long(m)), n(n_.begin(), n_.begin() + long(m));
    double a, b;
    if (!line(t, n, -1, a, b)) return false;
    double rss = 0, mean = 0, spread = 0;
    for (size_t i = 0; i < m; ++i) { const double e = t[i] - (a + b * n[i]); rss += e * e; mean += n[i]; }
    mean /= double(m);
    for (size_t i = 0; i < m; ++i) spread += (n[i] - mean) * (n[i] - mean);
    const double sigma = std::max(0.010, std::sqrt(rss / double(m - 2)));
    int late = 0, early = 0;
    double last = 0, limit = 0;
    for (size_t i = m; i < taps_.size(); ++i) {
      const double e = taps_[i] - (a + b * n_[i]);
      (e > 0 ? late : early)++;
      last = std::fabs(e);
      limit = 4.0 * sigma * std::sqrt(1.0 + 1.0 / double(m) + (n_[i] - mean) * (n_[i] - mean) / spread);
    }
    return (late == 4 || early == 4) && last > limit && last > 0.12 * b;
  }

  std::vector<double> taps_;
  Result result_;
  double a_ = 0, b_ = 0.5;
  std::vector<double> n_;
};

#endif /* TapTempo_hpp */
