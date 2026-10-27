// The trackers bpm_eval compares, behind one interface.
#pragma once

#include <memory>
#include <string>
#include <vector>

struct Tracker {
    virtual ~Tracker() = default;
    // One audio callback's worth of interleaved samples; `t` is the audio
    // time (seconds) of its first frame.
    virtual void process(const float* data, int frames, int channels, double sampleRate, double t) = 0;
    // What the app shows and drives visuals with, as of the last block.
    virtual double displayedBpm() const = 0;
    // The visual beat pulse's phase at time t (>= the last block's end):
    // 0 on a beat, rising towards 1 just before the next.
    virtual double pulsePhase(double t) const = 0;
    // Beat events reported since the last call, in audio time.
    virtual std::vector<double> takeBeats() = 0;
    // Someone tapped along: a tempo and a beat (audio time).
    virtual void hint(double /*bpm*/, double /*beatTime*/) {}
};

std::unique_ptr<Tracker> makeTracker(const std::string& name);
