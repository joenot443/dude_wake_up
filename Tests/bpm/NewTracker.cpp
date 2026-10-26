// The rebuilt tracker (src/Audio/BeatTracker.hpp), as the app uses it.
#include "Trackers.hpp"
#include "BeatTracker.hpp"

#include <cmath>
#include <cstdlib>

namespace {

class NewTracker : public Tracker {
public:
    void process(const float* data, int frames, int channels, double sampleRate, double) override {
        tracker_.process(data, frames, channels, sampleRate);
        state_ = tracker_.snapshot();
    }
    double displayedBpm() const override { return state_.bpm; }
    double pulsePhase(double t) const override {
        const double p = state_.phase + (t - state_.audioTime) * state_.bpm / 60.0;
        return p - std::floor(p);
    }
    std::vector<double> takeBeats() override { return tracker_.takeBeats(); }

private:
    BeatTracker tracker_;
    BeatTracker::Snapshot state_;
};

}  // namespace

// BT_MEM, BT_GAIN, BT_EVERY, BT_SNAP (seconds) override the tuning.
std::unique_ptr<Tracker> makeNewTracker() {
    auto& t = BeatTracker::tuning();
    auto env = [](const char* name, double& value) { if (const char* v = std::getenv(name)) value = std::atof(v); };
    double every = t.phaseEvery;
    env("BT_MEM", t.memory);
    env("BT_GAIN", t.phaseGain);
    env("BT_EVERY", every);
    env("BT_SNAP", t.snapSeconds);
    t.phaseEvery = std::max(1, int(every));
    return std::make_unique<NewTracker>();
}
