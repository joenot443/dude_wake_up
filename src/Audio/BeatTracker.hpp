//
//  BeatTracker.hpp
//  dude_wake_up
//
//  Real-time tempo and beat tracking for any audio source.
//
//  1. Input: interleaved audio at any rate and channel count is downmixed,
//     levelled (slow automatic gain, so quiet inputs track as well as loud
//     ones) and resampled to 22.05 kHz. Silence is fed too: time never stops.
//  2. Onsets: spectral flux of compressed (square-root) band energies, 24
//     log-spaced bands 40 Hz-10 kHz, a frame every 128 samples (5.8 ms).
//  3. Tempo, every 0.25 s: the autocorrelation of the last 8 s of onsets,
//     scored over each candidate period and its multiples (a comb, which
//     favours the beat over its subdivisions), weighted by a soft preference
//     for usual tempos, refined between frames for a precise BPM. Held with
//     hysteresis: small changes are smoothed, octave alternatives (x2, /2,
//     x3/2...) never flip the tempo, an unrelated tempo (a new track) takes
//     over once it has held for ~2.5 s.
//  3b. The beat level: among the comb's peaks and the leader's octave and
//     meter relatives, the period whose best-aligned pulse train lands on
//     the strongest onsets (salience), with the tempo preference settling
//     octaves.
//  4. Phase: that pulse-train alignment, ~29 times a second, steers a
//     phase-locked beat clock (half of each error corrected), which runs
//     on through breakdowns. The clock produces the beat events and the
//     beat phase the visuals use.
//
//  Measured with Tests/bpm (synthetic music with exact beats, and the
//  Ballroom dataset's 698 annotated recordings); see docs/BPM.md.
//
//  Header-only (so it needs no project-file entries); uses Gist's kiss_fft.
//  process() runs on the audio thread; snapshot() is safe from any thread.
//

#ifndef BeatTracker_hpp
#define BeatTracker_hpp

#include "kiss_fft.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <numeric>
#include <vector>

class BeatTracker {
public:
  struct Snapshot {
    double bpm = 120.0;       // tempo (precise; display rounding is the caller's)
    double confidence = 0.0;  // 0..1, how clear the beat is
    bool locked = false;      // a tempo has been found and is being followed
    double phase = 0.0;       // 0 on a beat, rising to 1 before the next, at audioTime
    double audioTime = 0.0;   // seconds of audio processed
    uint64_t beats = 0;       // beats so far
    std::chrono::steady_clock::time_point wallTime;  // when audioTime was processed
  };

  // Tempo range reported (others fold into it by octaves).
  static constexpr double kMinBpm = 60.0, kMaxBpm = 200.0;

  // Beat-clock tuning, chosen by sweeping Tests/bpm over the Ballroom set
  // (the app uses these defaults; the test program can override them).
  struct Tuning {
    double memory = 0.85;     // weight kept per older beat when lining up beats
    double phaseGain = 0.5;   // share of a phase error corrected per update
    int phaseEvery = 6;       // onset frames between phase updates (~29/s: smooth on screen)
    double snapSeconds = 0.75;  // a phase error over a quarter beat for this long: jump to it
  };
  static Tuning& tuning() { static Tuning t; return t; }

  BeatTracker() { reset(); }
  ~BeatTracker() {
    if (fft_) kiss_fft_free(fft_);
    if (acfFwd_) kiss_fft_free(acfFwd_);
    if (acfInv_) kiss_fft_free(acfInv_);
  }
  BeatTracker(const BeatTracker&) = delete;
  BeatTracker& operator=(const BeatTracker&) = delete;

  void reset() {
    inputRate_ = 0;
    gainEnv_ = 0.0;
    analysis_.clear();
    analysisStart_ = 0;
    framesDone_ = 0;
    prevBands_.assign(3, std::vector<float>(kBands, 0.f));
    odf_.clear();
    odfOffset_ = 0;
    tempo_ = 120.0;
    locked_ = false;
    candidate_ = 0; candidateCount_ = 0;
    alternative_ = 0; alternativeCount_ = 0;
    confidence_ = 0;
    clock_ = 0.0;
    audioTime_ = 0.0;
    phaseErrors_ = 0;
    snapPhase_ = true;
    beats_.clear();  // (beatCount_ carries on: consumers count on it rising)
    if (!fft_) {
      fft_ = kiss_fft_alloc(kFft, 0, nullptr, nullptr);
      acfFwd_ = kiss_fft_alloc(kAcfFft, 0, nullptr, nullptr);
      acfInv_ = kiss_fft_alloc(kAcfFft, 1, nullptr, nullptr);
      buildTables();
    }
    publish();
  }

  // Any thread: start over at the next buffer (new music: a track change).
  void requestReset() { resetRequested_.store(true); }

  // Audio thread: one callback's interleaved samples.
  void process(const float* data, int frames, int channels, double sampleRate) {
    if (frames <= 0 || channels <= 0 || sampleRate <= 0) return;
    if (resetRequested_.exchange(false)) reset();
    if (sampleRate != inputRate_) startResampler(sampleRate);

    // Downmix, then slow automatic gain (attack ~50 ms, release ~3 s) so
    // the analysis sees a steady level whatever the source's.
    mono_.resize(size_t(frames));
    for (int i = 0; i < frames; ++i) {
      float s = 0;
      for (int c = 0; c < channels; ++c) s += data[size_t(i) * size_t(channels) + size_t(c)];
      mono_[size_t(i)] = s / float(channels);
    }
    const double attack = 1.0 - std::exp(-1.0 / (0.05 * sampleRate));
    const double release = 1.0 - std::exp(-1.0 / (3.0 * sampleRate));
    for (float& s : mono_) {
      const double a = std::fabs(s);
      gainEnv_ += (a > gainEnv_ ? attack : release) * (a - gainEnv_);
      // Below about -70 dBFS it's silence: don't amplify noise into onsets.
      const double gain = gainEnv_ > 3e-4 ? std::min(200.0, 0.25 / gainEnv_) : 0.0;
      s = float(s * gain);
    }

    // To the analysis rate.
    resampler_.process(mono_.data(), frames, analysis_);

    // Onset frames, and the beat clock advanced through this block.
    const double blockStart = audioTime_;
    audioTime_ += frames / sampleRate;
    while (analysis_.size() - analysisStart_ >= size_t(kFft)) {
      analyzeFrame(&analysis_[analysisStart_]);
      analysisStart_ += kHop;
      ++framesDone_;
      if (framesDone_ % kUpdateFrames == 0) update();
      else if (locked_ && framesDone_ % uint64_t(tuning().phaseEvery) == 0) updatePhase();
    }
    if (analysisStart_ > 1 << 16) {
      analysis_.erase(analysis_.begin(), analysis_.begin() + long(analysisStart_));
      analysisStart_ = 0;
    }
    advanceClock(blockStart, audioTime_);
    publish();
  }

  Snapshot snapshot() const {
    std::lock_guard<std::mutex> lock(snapshotMutex_);
    return snapshot_;
  }

  // The beat phase `now`, carried on from a snapshot at its tempo.
  static double phaseAt(const Snapshot& s, std::chrono::steady_clock::time_point now) {
    const double elapsed = std::chrono::duration<double>(now - s.wallTime).count();
    const double p = s.phase + std::max(0.0, elapsed) * s.bpm / 60.0;
    return p - std::floor(p);
  }

  // Beat times (seconds of audio) since the last call. For testing.
  std::vector<double> takeBeats() {
    std::vector<double> out;
    out.swap(beats_);
    return out;
  }

private:
  static constexpr double kRate = 22050.0;
  static constexpr int kFft = 1024;
  static constexpr int kHop = 128;
  static constexpr double kFps = kRate / kHop;  // onset frames per second (172.3)
  static constexpr int kBands = 24;
  static constexpr int kUpdateFrames = 43;      // tempo/phase update every ~0.25 s
  static constexpr double kWindowSeconds = 8.0; // tempo evaluated over this much
  static constexpr double kMinSeconds = 3.0;    // before a first estimate
  static constexpr int kAcfFft = 4096;
  // Audio time of an onset frame relative to its first sample: where an
  // onset's flux peaks, a little past the middle of the analysis window.
  // Measured with Tests/bpm on synthetic beats (whose times are exact).
  static constexpr double kFrameLatency = (kFft / 2.0) / kRate + 0.016;

  // ---- Input -----------------------------------------------------------------

  // Band-limited resampling (windowed sinc, 16 taps each side, cut off
  // below the lower Nyquist). Output sample m is input time m / outRate
  // exactly: no delay to account for.
  class Resampler {
  public:
    void start(double inRate, double outRate) {
      step_ = inRate / outRate;
      cutoff_ = std::min(1.0, outRate / inRate) * 0.9;
      history_.assign(size_t(kTaps), 0.f);  // silence before the first sample
      pos_ = kTaps;
    }
    void process(const float* in, int n, std::vector<float>& out) {
      history_.insert(history_.end(), in, in + n);
      while (pos_ + kTaps + 1 < double(history_.size())) {
        const long i0 = long(std::floor(pos_));
        const double frac = pos_ - double(i0);
        double sum = 0;
        for (int k = -kTaps + 1; k <= kTaps; ++k) {
          const double x = double(k) - frac;
          const double w = 0.5 + 0.5 * std::cos(M_PI * x / kTaps);
          const double arg = M_PI * cutoff_ * x;
          const double sinc = std::fabs(arg) < 1e-9 ? 1.0 : std::sin(arg) / arg;
          sum += history_[size_t(i0 + k)] * cutoff_ * sinc * w;
        }
        out.push_back(float(sum));
        pos_ += step_;
      }
      const long drop = long(std::floor(pos_)) - kTaps;
      if (drop > 4096) {
        history_.erase(history_.begin(), history_.begin() + drop);
        pos_ -= double(drop);
      }
    }
  private:
    static constexpr int kTaps = 16;
    double step_ = 1, cutoff_ = 1, pos_ = 0;
    std::vector<float> history_;
  };

  void startResampler(double rate) {
    inputRate_ = rate;
    resampler_.start(rate, kRate);
  }

  void buildTables() {
    window_.resize(kFft);
    for (int i = 0; i < kFft; ++i) window_[size_t(i)] = float(0.5 - 0.5 * std::cos(2 * M_PI * i / kFft));
    // Triangular bands, log-spaced 40 Hz - 10 kHz.
    const int bins = kFft / 2 + 1;
    bandWeights_.assign(size_t(kBands), std::vector<float>(size_t(bins), 0.f));
    std::vector<double> edges(kBands + 2);
    for (int b = 0; b < kBands + 2; ++b) edges[size_t(b)] = 40.0 * std::pow(10000.0 / 40.0, double(b) / (kBands + 1));
    for (int b = 0; b < kBands; ++b) {
      for (int k = 0; k < bins; ++k) {
        const double f = k * kRate / kFft;
        const double lo = edges[size_t(b)], mid = edges[size_t(b) + 1], hi = edges[size_t(b) + 2];
        double w = 0;
        if (f > lo && f <= mid) w = (f - lo) / (mid - lo);
        else if (f > mid && f < hi) w = (hi - f) / (hi - mid);
        bandWeights_[size_t(b)][size_t(k)] = float(w);
      }
      // Low bands are narrower than a bin: give each at least its nearest bin.
      const int nearest = int(std::round(edges[size_t(b) + 1] * kFft / kRate));
      float& w = bandWeights_[size_t(b)][size_t(std::clamp(nearest, 0, bins - 1))];
      w = std::max(w, 1.0f);
    }
  }

  // ---- Onsets ----------------------------------------------------------------

  void analyzeFrame(const float* x) {
    for (int i = 0; i < kFft; ++i) { fftIn_[i].r = x[i] * window_[size_t(i)]; fftIn_[i].i = 0; }
    kiss_fft(fft_, fftIn_, fftOut_);
    const int bins = kFft / 2 + 1;
    for (int k = 0; k < bins; ++k) mag_[k] = std::sqrt(fftOut_[k].r * fftOut_[k].r + fftOut_[k].i * fftOut_[k].i);
    std::vector<float> bands(kBands);
    for (int b = 0; b < kBands; ++b) {
      double e = 0;
      const auto& w = bandWeights_[size_t(b)];
      for (int k = 0; k < bins; ++k) if (w[size_t(k)] > 0) e += w[size_t(k)] * mag_[k];
      // Square-root compression: evens out loud and quiet onsets less than
      // a logarithm, which made a soft note out of silence count like a kick.
      bands[size_t(b)] = float(std::sqrt(e));
    }
    // Rise against two frames back (11.6 ms).
    const auto& back = prevBands_[1];
    double flux = 0;
    for (int b = 0; b < kBands; ++b) flux += std::max(0.f, bands[size_t(b)] - back[size_t(b)]);
    prevBands_[1] = prevBands_[0];
    prevBands_[0] = bands;
    odf_.push_back(float(flux));
    const size_t keep = size_t(kFps * (kWindowSeconds + 2));
    if (odf_.size() > 2 * keep) {
      odfOffset_ += odf_.size() - keep;
      odf_.erase(odf_.begin(), odf_.end() - long(keep));
    }
  }

  // Audio time of onset frame n (absolute index).
  double frameTime(uint64_t n) const { return (double(n) * kHop) / kRate + kFrameLatency; }

  // ---- Tempo and phase -------------------------------------------------------

  void update() {
    const size_t available = odf_.size();
    if (available < size_t(kMinSeconds * kFps)) return;
    const size_t W = std::min(available, size_t(kWindowSeconds * kFps));
    const float* odf = odf_.data() + (available - W);

    // Peaks above a local mean (~0.25 s each side), half-wave rectified.
    std::vector<float> x(W);
    {
      const int half = int(kFps * 0.25);
      double sum = 0;
      int lo = 0, hi = -1;
      for (int i = 0; i < int(W); ++i) {
        while (hi < std::min(int(W) - 1, i + half)) sum += odf[++hi];
        while (lo < i - half) sum -= odf[lo++];
        const double mean = sum / (hi - lo + 1);
        x[size_t(i)] = float(std::max(0.0, odf[i] - mean));
      }
    }
    double energy = 0;
    for (float v : x) energy += double(v) * v;
    if (energy < 1e-9) { confidence_ = 0; return; }  // silence: hold everything

    // Autocorrelation (FFT), normalised for overlap.
    for (int i = 0; i < kAcfFft; ++i) { acfIn_[i].r = i < int(W) ? x[size_t(i)] : 0.f; acfIn_[i].i = 0; }
    kiss_fft(acfFwd_, acfIn_, acfOut_);
    for (int i = 0; i < kAcfFft; ++i) {
      acfOut_[i].r = acfOut_[i].r * acfOut_[i].r + acfOut_[i].i * acfOut_[i].i;
      acfOut_[i].i = 0;
    }
    kiss_fft(acfInv_, acfOut_, acfIn_);
    acf_.assign(W, 0.0);
    const double zero = acfIn_[0].r;
    for (size_t L = 0; L < W; ++L) acf_[L] = acfIn_[L].r / zero * double(W) / double(W - L);

    // Comb score over periods: the period and its first multiples.
    const double minLag = 60.0 * kFps / kMaxBpm, maxLag = 60.0 * kFps / kMinBpm;
    const double step = 0.25;
    std::vector<double> lags, scores;
    for (double L = minLag; L <= maxLag; L += step) {
      double s = 0;
      int terms = 0;
      for (int k = 1; k <= 4; ++k) {
        if (k * L >= W * 0.6) break;
        s += acfAt(k * L);
        ++terms;
      }
      if (terms == 0) continue;
      lags.push_back(L);
      scores.push_back(s / terms);
    }
    if (scores.size() < 3) return;

    // How clear the beat is: the best comb score against the typical one.
    {
      std::vector<double> sorted = scores;
      std::nth_element(sorted.begin(), sorted.begin() + long(sorted.size() / 2), sorted.end());
      const double typical = sorted[sorted.size() / 2];
      const double top = *std::max_element(scores.begin(), scores.end());
      confidence_ = std::clamp((top - typical) / (std::fabs(top) + 1e-9), 0.0, 1.0);
    }
    if (confidence_ < 0.15) return;  // no clear beat (breakdown, ambience): hold

    // Candidates: the comb's strongest peaks, and the leader's octave and
    // meter relatives, which the comb can't tell apart (they share
    // periodicities). Each is then judged on the beat level itself.
    smoothOnsets(odf, W);
    std::vector<size_t> peaks;
    for (size_t i = 1; i + 1 < scores.size(); ++i)
      if (scores[i] >= scores[i - 1] && scores[i] >= scores[i + 1] && scores[i] > 0) peaks.push_back(i);
    std::sort(peaks.begin(), peaks.end(), [&](size_t a, size_t b) { return scores[a] > scores[b]; });
    if (peaks.size() > 5) peaks.resize(5);
    std::vector<double> candidates;
    for (size_t i : peaks) candidates.push_back(refineLag(lags[i], W));
    if (!candidates.empty()) {
      const double lead = candidates[0];
      for (double r : {2.0, 0.5, 1.5, 2.0 / 3.0, 3.0, 1.0 / 3.0, 4.0 / 3.0, 0.75}) {
        const double L = lead * r;
        if (L >= minLag && L <= maxLag) candidates.push_back(refineLag(L, W));
      }
    }
    if (locked_) candidates.push_back(refineLag(60.0 * kFps / tempo_, W));
    Beat best;
    for (double L : candidates) {
      const Beat b = judge(L, W);
      if (b.score > best.score) best = b;
    }
    if (best.score <= 0) return;
    const double estimate = 60.0 * kFps / best.period;
#ifdef BEATTRACKER_TRACE
    std::printf("t %6.2f est %6.2f score %.3f conf %.2f |", audioTime_, estimate, best.score, confidence_);
    for (double L : candidates) { const Beat b = judge(L, W); std::printf(" %.1f:%.3f", 60.0 * kFps / L, b.score); }
    std::printf(" | tempo %6.2f%s\n", tempo_, locked_ ? " L" : "");
#endif

    followTempo(estimate, best.score, W);
    if (locked_) updatePhase(false);
  }

  // Phase from the tempo being followed (which may not be the estimate).
  void updatePhase(bool fresh = true) {
    const size_t available = odf_.size();
    if (available < size_t(kMinSeconds * kFps)) return;
    const size_t W = std::min(available, size_t(kWindowSeconds * kFps));
    if (fresh) smoothOnsets(odf_.data() + (available - W), W);
    const Beat current = judge(60.0 * kFps / tempo_, W);
    followPhase(current.phase);
  }

  // A candidate beat period (frames), how well it fits as *the* beat, and
  // where its beats fall (frames before the latest onset frame).
  struct Beat { double period = 0, score = -1, phase = 0; };

  // The current window's onsets, smoothed (Gaussian, sigma 1.5 frames).
  // (Weighting kick/bass onsets more, to anchor beats on them, made phase
  // worse on real recordings: bass lines are often syncopated.)
  void smoothOnsets(const float* odf, size_t W) {
    smooth_.assign(W, 0.f);
    const int r = 4;
    double mean = 0;
    for (int i = 0; i < int(W); ++i) {
      double s = 0, w = 0;
      for (int j = -r; j <= r; ++j) {
        const int k = i + j;
        if (k < 0 || k >= int(W)) continue;
        const double g = std::exp(-0.5 * (j / 1.5) * (j / 1.5));
        s += g * odf[k];
        w += g;
      }
      smooth_[size_t(i)] = float(s / w);
      mean += s / w;
    }
    smoothMean_ = mean / double(W) + 1e-9;
  }

  double smoothAt(double pos) const {
    if (pos < 0 || pos >= double(smooth_.size() - 1)) return 0.0;
    const size_t i = size_t(pos);
    const double f = pos - double(i);
    return smooth_[i] * (1 - f) + smooth_[i + 1] * f;
  }

  // Lines a pulse train of `period` up with the recent onsets (recent beats
  // counting most) and scores it as the beat level.
  Beat judge(double period, size_t W) const {
    Beat b;
    b.period = period;
    const double last = double(W - 1);
    const int count = std::max(2, int((W - 1) / period));
    double bestOn = -1, bestPhi = 0;
    std::vector<double> phiScores;
    for (double phi = 0; phi < period; phi += 0.5) {
      double s = 0, w = 1;
      for (int k = 0; k < count; ++k) { s += w * smoothAt(last - phi - k * period); w *= tuning().memory; }
      phiScores.push_back(s);
      if (s > bestOn) { bestOn = s; bestPhi = phi; }
    }
    const size_t bi = size_t(bestPhi / 0.5);
    if (bi > 0 && bi + 1 < phiScores.size()) {
      const double a = phiScores[bi - 1], c0 = phiScores[bi], c = phiScores[bi + 1];
      const double d = a - 2 * c0 + c;
      if (d < 0) bestPhi += 0.5 * std::clamp(0.5 * (a - c) / d, -0.5, 0.5);
    }
    double on = 0, off = 0, norm = 0, w = 1;
    for (int k = 0; k < count; ++k) {
      const double beat = last - bestPhi - k * period;
      on += w * smoothAt(beat);
      off += w * smoothAt(beat + period / 2);
      norm += w;
      w *= tuning().memory;
    }
    // Salience: how strong the onsets on these beats are against onsets in
    // general. A grid at 2/3 or 3/2 of the beat lands between beats half the
    // time, and one at double the tempo on weak subdivisions: both score
    // low. Half the tempo scores like the tempo itself (its beats are a
    // subset), so the tempo preference settles octaves.
    (void)off;
    const double salience = on / norm / smoothMean_;
    b.score = salience * prior(60.0 * kFps / period);
    b.phase = bestPhi;
    return b;
  }

  double acfAt(double L) const {
    const size_t i = size_t(L);
    if (i + 1 >= acf_.size()) return 0;
    const double f = L - double(i);
    return acf_[i] * (1 - f) + acf_[i + 1] * f;
  }

  // The lag from where the multiples of `lag` actually peak.
  double refineLag(double lag, size_t W) const {
    double num = 0, den = 0;
    for (int k = 1; k <= 4; ++k) {
      const double c = k * lag;
      if (c + 3 >= W * 0.6) break;
      // Peak within +-1.5 frames of k*lag, to sub-frame precision.
      int i = int(std::round(c));
      for (int j = int(std::round(c - 1.5)); j <= int(std::round(c + 1.5)); ++j)
        if (j > 0 && size_t(j) + 1 < acf_.size() && acf_[size_t(j)] > acf_[size_t(i)]) i = j;
      if (i <= 0 || size_t(i) + 1 >= acf_.size()) continue;
      const double a = acf_[size_t(i) - 1], b = acf_[size_t(i)], d = acf_[size_t(i) + 1];
      const double curv = a - 2 * b + d;
      const double peak = curv < 0 ? i + std::clamp(0.5 * (a - d) / curv, -0.5, 0.5) : double(i);
      if (std::fabs(peak / k - lag) > 1.0) continue;  // not this multiple's peak
      num += peak * k;  // weight later multiples more (finer)
      den += double(k) * k;
    }
    return den > 0 ? num / den : lag;
  }

  // Soft preference for usual tempos: log-normal around 120 BPM, about an
  // octave wide.
  static double prior(double bpm) {
    const double x = std::log2(bpm / 120.0) / 0.9;
    return std::exp(-0.5 * x * x);
  }

  void followTempo(double estimate, double score, size_t W) {
    if (!locked_) {
      if (candidate_ > 0 && std::fabs(estimate / candidate_ - 1) < 0.02) {
        candidate_ = 0.5 * (candidate_ + estimate);
        if (++candidateCount_ >= 3) {
          tempo_ = candidate_;
          locked_ = true;
          snapPhase_ = true;
          alternative_ = 0; alternativeCount_ = 0;
        }
      } else {
        candidate_ = estimate;
        candidateCount_ = 1;
      }
      return;
    }
    const double ratio = estimate / tempo_;
    if (std::fabs(ratio - 1) < 0.04) {
      // Same tempo: follow it smoothly (time constant ~1.2 s).
      tempo_ *= std::pow(ratio, 0.2);
      alternative_ = 0; alternativeCount_ = 0;
      return;
    }
    // Another tempo (a better octave, or a new track) wins only by beating
    // ours clearly for ~2 s. (Holding octave/meter relatives back longer,
    // against passing triplet figures, measured worse: octave corrections
    // came later.)
    const double ours = judge(refineLag(60.0 * kFps / tempo_, W), W).score;
    if (score < 1.2 * ours) { alternative_ = 0; alternativeCount_ = 0; return; }
    if (alternative_ > 0 && std::fabs(estimate / alternative_ - 1) < 0.03) {
      alternative_ = 0.5 * (alternative_ + estimate);
      if (++alternativeCount_ >= 8) {
        tempo_ = alternative_;
        snapPhase_ = true;
        alternative_ = 0; alternativeCount_ = 0;
      }
    } else {
      alternative_ = estimate;
      alternativeCount_ = 1;
    }
  }

  // Follows where the beats fall (`phase`: frames before the latest onset
  // frame to the last beat) with the beat clock.
  void followPhase(double phase) {
    const uint64_t lastFrame = odfOffset_ + odf_.size() - 1;
    const double lastBeat = frameTime(lastFrame) - phase / kFps;
    const double beatSeconds = 60.0 / tempo_;
    double measured = (audioTime_ - lastBeat) / beatSeconds;
    measured -= std::floor(measured);
    double error = measured - (clock_ - std::floor(clock_));
    if (error > 0.5) error -= 1;
    if (error < -0.5) error += 1;
    const int snapAfter = std::max(1, int(tuning().snapSeconds * kFps / tuning().phaseEvery));
    if (snapPhase_ || (std::fabs(error) > 0.25 && ++phaseErrors_ >= snapAfter)) {
      setClockFraction(measured);
      snapPhase_ = false;
      phaseErrors_ = 0;
    } else {
      if (std::fabs(error) <= 0.25) phaseErrors_ = 0;
      clock_ += tuning().phaseGain * error;  // gentle correction
    }
  }

  // Moves the clock's phase within the current beat (never across one, so
  // a snap can't repeat or skip a beat event).
  void setClockFraction(double fraction) {
    const double whole = std::floor(clock_);
    double next = whole + fraction;
    if (next < clock_ - 0.5) next += 1;  // forwards past a beat that would've just fired
    if (next > clock_ + 0.5) next -= 1;
    clock_ = std::max(next, std::floor(clock_ - 1e-9));
  }

  void advanceClock(double from, double to) {
    const double beatsPerSecond = tempo_ / 60.0;
    const double start = clock_;
    clock_ += (to - from) * beatsPerSecond;
    // A beat event for each whole beat crossed, at its audio time.
    for (double b = std::floor(start) + 1; b <= clock_; b += 1) {
      beats_.push_back(from + (b - start) / beatsPerSecond);
      ++beatCount_;
    }
  }

  void publish() {
    std::lock_guard<std::mutex> lock(snapshotMutex_);
    snapshot_.bpm = tempo_;
    snapshot_.confidence = confidence_;
    snapshot_.locked = locked_;
    snapshot_.phase = clock_ - std::floor(clock_);
    snapshot_.audioTime = audioTime_;
    snapshot_.beats = beatCount_;
    snapshot_.wallTime = std::chrono::steady_clock::now();
  }

  // Input
  Resampler resampler_;
  double inputRate_ = 0;
  double gainEnv_ = 0;
  std::vector<float> mono_, resampled_, analysis_;
  size_t analysisStart_ = 0;
  uint64_t framesDone_ = 0;

  // Onsets
  kiss_fft_cfg fft_ = nullptr, acfFwd_ = nullptr, acfInv_ = nullptr;
  kiss_fft_cpx fftIn_[kFft], fftOut_[kFft];
  kiss_fft_cpx acfIn_[kAcfFft], acfOut_[kAcfFft];
  float mag_[kFft / 2 + 1];
  std::vector<float> window_;
  std::vector<std::vector<float>> bandWeights_, prevBands_;
  std::vector<float> odf_;
  uint64_t odfOffset_ = 0;  // absolute index of odf_[0]
  std::vector<double> acf_;
  std::vector<float> smooth_;
  double smoothMean_ = 1;

  // Tempo and phase
  double tempo_ = 120.0;
  bool locked_ = false;
  double candidate_ = 0, alternative_ = 0;
  int candidateCount_ = 0, alternativeCount_ = 0;
  double confidence_ = 0;
  double clock_ = 0;        // beats (whole and fraction) at audioTime_
  double audioTime_ = 0;
  int phaseErrors_ = 0;
  bool snapPhase_ = true;
  uint64_t beatCount_ = 0;
  std::vector<double> beats_;

  std::atomic<bool> resetRequested_{false};
  mutable std::mutex snapshotMutex_;
  Snapshot snapshot_;
};

#endif /* BeatTracker_hpp */
