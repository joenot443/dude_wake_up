//
//  AutoBpmDetector.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2025-09-06.
//

#ifndef AutoBpmDetector_hpp
#define AutoBpmDetector_hpp

#include "Gist.h"
#include <memory>
#include <vector>
#include <deque>
#include <chrono>

struct SimpleBeat {
    float bpm;
    bool isBeat;
    
    SimpleBeat(float b = 120.0f, bool beat = false) : bpm(b), isBeat(beat) {}
};

class AutoBpmDetector {
private:
    std::size_t frameSampleCount;
    float samplingFrequency;
    
    // Onset detection
    float onsetThreshold;
    float lastOnsetValue;
    std::deque<float> onsetHistory;
    std::deque<std::chrono::steady_clock::time_point> beatTimes;
    
    // BPM calculation
    float currentBpm;
    bool beatDetected;
    std::chrono::steady_clock::time_point lastBeatTime;
    
    // Smoothing
    std::deque<float> bpmHistory;
    static const size_t MAX_BPM_HISTORY = 8;
    static const size_t MAX_BEAT_TIMES = 8;
    
    // BPM update control
    size_t beatCount;
    static const size_t BEATS_BEFORE_UPDATE = 10;
    
public:
    AutoBpmDetector(float samplingFrequency = 44100, std::size_t frameSampleCount = 512);
    ~AutoBpmDetector();
    
    SimpleBeat processFrame(Gist<float>* gist);
    
    float getCurrentBpm() const;
    bool isBeatDetected() const;
    SimpleBeat getLastBeat() const;
    void resetBeatTracking();
    
private:
    bool detectOnset(float onsetValue);
    void updateBpmFromBeats();
    float smoothBpm(float newBpm);
    bool isValidBpm(float bpm) const;
    
    SimpleBeat lastBeat;
};


#endif /* AutoBpmDetector_hpp */