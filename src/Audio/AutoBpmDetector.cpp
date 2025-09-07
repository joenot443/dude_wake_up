//
//  AutoBpmDetector.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2025-09-06.
//

#include "AutoBpmDetector.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

AutoBpmDetector::AutoBpmDetector(float samplingFrequency, std::size_t frameSampleCount) 
    : samplingFrequency(samplingFrequency), 
      frameSampleCount(frameSampleCount),
      onsetThreshold(0.1f),
      lastOnsetValue(0.0f),
      currentBpm(120.0f),
      beatDetected(false),
      beatCount(0) {
    
    lastBeat = SimpleBeat(120.0f, false);
    lastBeatTime = std::chrono::steady_clock::now();
}

AutoBpmDetector::~AutoBpmDetector() = default;

SimpleBeat AutoBpmDetector::processFrame(Gist<float>* gist) {
    if (!gist) {
        return lastBeat;
    }
    
    // Get onset detection value from Gist (using spectral difference)
    float onsetValue = gist->spectralDifferenceHWR();
    
    // Detect beat/onset
    bool onset = detectOnset(onsetValue);
    
    if (onset) {
        auto currentTime = std::chrono::steady_clock::now();
        beatTimes.push_back(currentTime);
        beatCount++;
        
        // Limit history size
        if (beatTimes.size() > MAX_BEAT_TIMES) {
            beatTimes.pop_front();
        }
        
        // Only update BPM every 10 beats
        if (beatCount >= BEATS_BEFORE_UPDATE) {
            updateBpmFromBeats();
            beatCount = 0; // Reset counter
        }
        
        lastBeatTime = currentTime;
        beatDetected = true;
    } else {
        beatDetected = false;
    }
    
    lastBeat = SimpleBeat(currentBpm, beatDetected);
    return lastBeat;
}

bool AutoBpmDetector::detectOnset(float onsetValue) {
    // Add to history
    onsetHistory.push_back(onsetValue);
    if (onsetHistory.size() > 20) { // Keep last 20 frames for comparison
        onsetHistory.pop_front();
    }
    
    if (onsetHistory.size() < 5) {
        return false; // Need some history
    }
    
    // Calculate adaptive threshold
    float meanOnset = 0.0f;
    for (float val : onsetHistory) {
        meanOnset += val;
    }
    meanOnset /= onsetHistory.size();
    
    float adaptiveThreshold = meanOnset + (onsetThreshold * 2.0f);
    
    // Check if current value is significantly higher than recent past
    bool isPeak = onsetValue > adaptiveThreshold && onsetValue > lastOnsetValue;
    
    // Prevent multiple detections too close together (minimum 200ms between beats)
    auto currentTime = std::chrono::steady_clock::now();
    auto timeSinceLastBeat = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastBeatTime).count();
    
    if (isPeak && timeSinceLastBeat > 200) {
        lastOnsetValue = onsetValue;
        return true;
    }
    
    lastOnsetValue = onsetValue;
    return false;
}

void AutoBpmDetector::updateBpmFromBeats() {
    if (beatTimes.size() < 2) {
        return; // Need at least 2 beats to calculate BPM
    }
    
    // Calculate intervals between recent beats
    std::vector<float> intervals;
    for (size_t i = 1; i < beatTimes.size(); ++i) {
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(
            beatTimes[i] - beatTimes[i-1]).count();
        
        if (interval > 0) {
            // Here's the key fix: double the interval to get the actual tempo
            float bpm = 60000.0f / (interval * 2.0f); // Multiply interval by 2
            if (isValidBpm(bpm)) {
                intervals.push_back(bpm);
            }
        }
    }
    
    if (intervals.empty()) {
        return;
    }
    
    // Calculate median BPM from recent intervals
    std::sort(intervals.begin(), intervals.end());
    float medianBpm;
    size_t n = intervals.size();
    if (n % 2 == 0) {
        medianBpm = (intervals[n/2 - 1] + intervals[n/2]) / 2.0f;
    } else {
        medianBpm = intervals[n/2];
    }
    
    // Apply smoothing
    currentBpm = smoothBpm(medianBpm);
}


float AutoBpmDetector::smoothBpm(float newBpm) {
    bpmHistory.push_back(newBpm);
    if (bpmHistory.size() > MAX_BPM_HISTORY) {
        bpmHistory.pop_front();
    }
    
    // Calculate weighted average (recent values weighted more)
    float sum = 0.0f;
    float weightSum = 0.0f;
    
    for (size_t i = 0; i < bpmHistory.size(); ++i) {
        float weight = 1.0f + (i * 0.5f); // Newer values get higher weight
        sum += bpmHistory[i] * weight;
        weightSum += weight;
    }
    
    return weightSum > 0.0f ? sum / weightSum : currentBpm;
}

bool AutoBpmDetector::isValidBpm(float bpm) const {
    return bpm >= 60.0f && bpm <= 200.0f;
}

float AutoBpmDetector::getCurrentBpm() const {
    return currentBpm;
}

bool AutoBpmDetector::isBeatDetected() const {
    return beatDetected;
}

SimpleBeat AutoBpmDetector::getLastBeat() const {
    return lastBeat;
}

void AutoBpmDetector::resetBeatTracking() {
    beatTimes.clear();
    bpmHistory.clear();
    onsetHistory.clear();
    currentBpm = 120.0f;
    beatDetected = false;
    beatCount = 0;
    lastBeat = SimpleBeat(120.0f, false);
}