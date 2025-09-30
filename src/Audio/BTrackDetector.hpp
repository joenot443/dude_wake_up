//
//  BTrackDetector.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2025-09-07.
//

#ifndef BTrackDetector_hpp
#define BTrackDetector_hpp

#include "BTrack.h"
#include "Gist.h"
#include <memory>
#include <vector>
#include <chrono>

struct SimpleBeat {
    float bpm;
    bool isBeat;
    
    SimpleBeat(float b = 120.0f, bool beat = false) : bpm(b), isBeat(beat) {}
};

class BTrackDetector {
private:
    std::unique_ptr<BTrack> btrack;
    SimpleBeat lastBeat;
    std::vector<double> audioBuffer;
    float samplingFrequency;
    size_t frameSampleCount;

    // Buffer for accumulating samples (BTrack needs 1024 samples)
    std::vector<float> sampleBuffer;
    size_t samplesInBuffer;
    static const size_t BTRACK_FRAME_SIZE = 1024;

    // BPM smoothing
    std::vector<float> bpmHistory;
    static const size_t MAX_BPM_HISTORY = 5;
    
    // Convert float vector to double array for BTrack
    void convertFloatVectorToDoubleArray(const std::vector<float>& input, std::vector<double>& output);
    
    // Smooth BPM values
    float smoothBpm(float newBpm);
    
    // Validate BPM range
    bool isValidBpm(float bpm) const;
    
public:
    BTrackDetector(float samplingFrequency = 44100, size_t frameSampleCount = 512);
    ~BTrackDetector();
    
    SimpleBeat processFrame(Gist<float>* gist);
    SimpleBeat processAudioFrame(const std::vector<float>& frame);
    
    float getCurrentBpm() const;
    bool isBeatDetected() const;
    SimpleBeat getLastBeat() const;
    void resetBeatTracking();
    
    // BTrack-specific methods
    void setTempo(double tempo);
    void fixTempo(double tempo);
    void doNotFixTempo();
};

#endif /* BTrackDetector_hpp */
