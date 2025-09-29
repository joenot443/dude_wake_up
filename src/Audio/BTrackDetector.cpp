//
//  BTrackDetector.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2025-09-07.
//

#include "BTrackDetector.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

BTrackDetector::BTrackDetector(float samplingFrequency, size_t frameSampleCount) 
    : samplingFrequency(samplingFrequency), 
      frameSampleCount(frameSampleCount),
      lastBeat(120.0f, false) {
    
    // Initialize BTrack with the provided frame size
    btrack = std::make_unique<BTrack>(static_cast<int>(frameSampleCount));
    
    // Reserve space for audio buffer conversion
    audioBuffer.reserve(frameSampleCount);
    
    // Initialize with reasonable default tempo
    btrack->setTempo(120.0);
}

BTrackDetector::~BTrackDetector() = default;

SimpleBeat BTrackDetector::processFrame(Gist<float>* gist) {
    if (!gist || !btrack) {
        return lastBeat;
    }
    
    // Get the raw audio frame from Gist's input buffer
    // Note: This assumes Gist stores the input audio frame
    // We need to get the audio data that was used to compute the spectral analysis
    
    // For now, we'll use the onset detection approach
    // Get onset detection value from Gist (using spectral difference)
    float onsetValue = gist->spectralDifferenceHWR();
    
    // Convert to double and process as onset detection function sample
    double onsetSample = static_cast<double>(onsetValue);
  std::vector<double> tmp(gist->audioFrame.begin(), gist->audioFrame.end());
      btrack->processOnsetDetectionFunctionSample(onsetSample);
  btrack->processAudioFrame(tmp.data());

  // 2) feed onset value derived from that frame
  btrack->processOnsetDetectionFunctionSample(onsetSample);

  // 3) optionally update tempo, then query results
  btrack->calculateTempo();
  const bool beatDetected = btrack->beatDueInCurrentFrame();
  const float currentBpm = static_cast<float>(btrack->getCurrentTempoEstimate());

    // Apply BPM validation and smoothing
//    if (isValidBpm(currentBpm)) {
//        currentBpm = smoothBpm(currentBpm);
//    } else {
//        // Keep the last valid BPM if current estimate is invalid
//        currentBpm = lastBeat.bpm;
//    }
    
    lastBeat = SimpleBeat(currentBpm, beatDetected);
    return lastBeat;
}

SimpleBeat BTrackDetector::processAudioFrame(const std::vector<float>& frame) {
    if (!btrack || frame.size() != frameSampleCount) {
        return lastBeat;
    }
    
    // Convert float vector to double array
    convertFloatVectorToDoubleArray(frame, audioBuffer);
    
    // Process the audio frame
    btrack->processAudioFrame(audioBuffer.data());
    
    // Get beat detection result
    bool beatDetected = btrack->beatDueInCurrentFrame();
    float currentBpm = static_cast<float>(btrack->getCurrentTempoEstimate());
    
    // Apply BPM validation and smoothing
    if (isValidBpm(currentBpm)) {
        currentBpm = smoothBpm(currentBpm);
    } else {
        currentBpm = lastBeat.bpm;
    }
    
    lastBeat = SimpleBeat(currentBpm, beatDetected);
    return lastBeat;
}

void BTrackDetector::convertFloatVectorToDoubleArray(const std::vector<float>& input, std::vector<double>& output) {
    output.resize(input.size());
    std::transform(input.begin(), input.end(), output.begin(), 
                   [](float f) { return static_cast<double>(f); });
}

float BTrackDetector::smoothBpm(float newBpm) {
    bpmHistory.push_back(newBpm);
    
    // Keep history size limited
    if (bpmHistory.size() > MAX_BPM_HISTORY) {
        bpmHistory.erase(bpmHistory.begin());
    }
    
    // Calculate weighted average (recent values weighted more)
    float sum = 0.0f;
    float weightSum = 0.0f;
    
    for (size_t i = 0; i < bpmHistory.size(); ++i) {
        float weight = 1.0f + (static_cast<float>(i) * 0.3f); // Newer values get higher weight
        sum += bpmHistory[i] * weight;
        weightSum += weight;
    }
    
    return weightSum > 0.0f ? sum / weightSum : newBpm;
}

bool BTrackDetector::isValidBpm(float bpm) const {
    return bpm >= 60.0f && bpm <= 200.0f;
}

float BTrackDetector::getCurrentBpm() const {
    return lastBeat.bpm;
}

bool BTrackDetector::isBeatDetected() const {
    return lastBeat.isBeat;
}

SimpleBeat BTrackDetector::getLastBeat() const {
    return lastBeat;
}

void BTrackDetector::resetBeatTracking() {
    bpmHistory.clear();
    lastBeat = SimpleBeat(120.0f, false);
    
    if (btrack) {
        // Reinitialize BTrack to reset internal state
        btrack = std::make_unique<BTrack>(static_cast<int>(frameSampleCount));
        btrack->setTempo(120.0);
    }
}

void BTrackDetector::setTempo(double tempo) {
    if (btrack) {
        btrack->setTempo(tempo);
    }
}

void BTrackDetector::fixTempo(double tempo) {
    if (btrack) {
        btrack->fixTempo(tempo);
    }
}

void BTrackDetector::doNotFixTempo() {
    if (btrack) {
        btrack->doNotFixTempo();
    }
}
