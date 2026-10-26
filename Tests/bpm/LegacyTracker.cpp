// The BPM path as it shipped before the rebuild, replicated around the real
// BTrackDetector: AudioSource::audioIn/processFrame (silence gate, 512-sample
// chunks of the interleaved buffer, debounced beats) and
// AudioSourceService::update (shown BPM = the detector's, beat pulse from a
// clock re-anchored to the last beat).
#include "Trackers.hpp"
#include "BTrackDetector.hpp"

#include <algorithm>
#include <cmath>

namespace {

class LegacyTracker : public Tracker {
public:
    void process(const float* data, int frames, int channels, double sampleRate, double t) override {
        const size_t n = size_t(frames) * size_t(channels);
        float avg = 0;
        for (size_t i = 0; i < n; ++i) avg += std::fabs(data[i]);
        avg /= float(n);
        if (avg < 0.001f) return;  // "mostly silence": dropped
        // The callback's time ("now" in the app) is when the block has arrived.
        const double now = t + frames / sampleRate;
        accumulator_.insert(accumulator_.end(), data, data + n);  // interleaved, as audioIn passes it
        if (accumulator_.size() > 512 * 8) accumulator_.erase(accumulator_.begin(), accumulator_.end() - 512);
        while (accumulator_.size() >= 512) {
            std::vector<float> chunk(accumulator_.begin(), accumulator_.begin() + 512);
            SimpleBeat beat = detector_.processAudioFrame(chunk);
            if (beat.bpm > 0 && beat.bpm >= 50 && beat.bpm <= 200) bpm_ = beat.bpm;
            if (beat.isBeat) {
                const double minInterval = 36.0 / std::max(1.0, bpm_);  // 60% of a beat
                if (now - lastBeat_ > minInterval) {
                    lastBeat_ = now;
                    beats_.push_back(now);
                }
            }
            accumulator_.erase(accumulator_.begin(), accumulator_.begin() + 512);
        }
    }
    double displayedBpm() const override { return bpm_; }
    double pulsePhase(double t) const override {
        // bpmStartTime follows the last beat; bpmPct() is the phase since it.
        const double period = 60.0 / bpm_;
        double pct = std::fmod(t - lastBeat_, period) / period;
        if (pct < 0) pct += 1;
        return pct;
    }
    std::vector<double> takeBeats() override { return std::exchange(beats_, {}); }

private:
    BTrackDetector detector_;
    std::vector<float> accumulator_;
    double bpm_ = 120.0;
    double lastBeat_ = -1000.0;
    std::vector<double> beats_;
};

}  // namespace

std::unique_ptr<Tracker> makeLegacyTracker() { return std::make_unique<LegacyTracker>(); }
