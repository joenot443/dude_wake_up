//
//  BTrackDetector.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2025-09-07.
//

#include "BTrackDetector.hpp"
#include "Console.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

BTrackDetector::BTrackDetector(float samplingFrequency, size_t frameSampleCount)
: samplingFrequency(samplingFrequency),
frameSampleCount(frameSampleCount),
lastBeat(120.0f, false),
samplesInBuffer(0) {

  // Initialize BTrack with hop size 512, which will use frame size 1024
  btrack = std::make_unique<BTrack>(512);

  // Reserve space for audio buffer conversion and sample buffering
  audioBuffer.reserve(BTRACK_FRAME_SIZE);
  sampleBuffer.resize(BTRACK_FRAME_SIZE, 0.0f);  // Initialize to zero!

  // Feed several frames of zeros to clear out initialization artifacts
  std::vector<double> zeros(BTRACK_FRAME_SIZE, 0.0);
  for (int i = 0; i < 10; i++) {
    btrack->processAudioFrame(zeros.data());
  }
}

BTrackDetector::~BTrackDetector() = default;

SimpleBeat BTrackDetector::processAudioFrame(const std::vector<float>& audioFrame) {
  if (!btrack) {
    return lastBeat;
  }

  // BTrack needs 1024 samples but we receive 512 at a time
  // Buffer the samples until we have enough

  // Copy new samples into buffer
  for (size_t i = 0; i < audioFrame.size() && samplesInBuffer < BTRACK_FRAME_SIZE; i++) {
    sampleBuffer[samplesInBuffer++] = audioFrame[i];
  }

  // Only process when we have a full 1024-sample frame
  if (samplesInBuffer < BTRACK_FRAME_SIZE) {
    return lastBeat;  // Not enough samples yet
  }

  // Convert to double and process
  std::vector<double> tmp(sampleBuffer.begin(), sampleBuffer.begin() + BTRACK_FRAME_SIZE);

  // Debug: Check audio levels
  static int frameCount = 0;
  if (frameCount++ % 20 == 0) {  // Log every 20th frame
    double maxVal = 0, avgVal = 0;
    for (size_t i = 0; i < tmp.size(); i++) {
      avgVal += std::abs(tmp[i]);
      maxVal = std::max(maxVal, std::abs(tmp[i]));
    }
    avgVal /= tmp.size();
    std::cout << "Audio frame: max=" << maxVal << " avg=" << avgVal << std::endl;
  }

  btrack->processAudioFrame(tmp.data());

  // Shift buffer: move last 512 samples to beginning (50% overlap)
  std::copy(sampleBuffer.begin() + 512, sampleBuffer.begin() + BTRACK_FRAME_SIZE, sampleBuffer.begin());
  samplesInBuffer = 512;

  // Query results - tempo calculation happens automatically inside BTrack
  const bool beatDetected = btrack->beatDueInCurrentFrame();
  const float currentBpm = static_cast<float>(btrack->getCurrentTempoEstimate());

  // BTrack's calculateTempo() is broken - always returns 78.30 BPM
  // Measure actual BPM from beat intervals instead
  static int lastBeatFrame = -1;
  static std::vector<float> beatIntervals;

  float measuredBpm = lastBeat.bpm; // Keep previous BPM until we have a new measurement

  if (beatDetected) {
    if (lastBeatFrame >= 0) {
      int framesSinceLast = frameCount - lastBeatFrame;
      // Calculate BPM from frame interval
      // frames * (hopSize / sampleRate) = seconds between beats
      // BPM = 60 / seconds
      float secondsSinceLast = framesSinceLast * (512.0f / 44100.0f);
      float intervalBpm = 60.0f / secondsSinceLast;

      // Only accept reasonable BPM values (40-200 BPM)
      if (intervalBpm >= 40.0f && intervalBpm <= 200.0f) {
        beatIntervals.push_back(intervalBpm);
        if (beatIntervals.size() > 8) {
          beatIntervals.erase(beatIntervals.begin());
        }

        // Average the last few beat intervals with exponential weighting
        float sum = 0;
        float weightSum = 0;
        for (size_t i = 0; i < beatIntervals.size(); i++) {
          float weight = 1.0f + (i * 0.5f); // More recent values weighted higher
          sum += beatIntervals[i] * weight;
          weightSum += weight;
        }
        measuredBpm = sum / weightSum;
      }
    }
    lastBeatFrame = frameCount;
  }
  frameCount++;

  lastBeat = SimpleBeat(measuredBpm, beatDetected);

  return lastBeat;
}

SimpleBeat BTrackDetector::processFrame(Gist<float>* gist) {
  if (!gist) {
    return lastBeat;
  }
  // Delegate to processAudioFrame with gist's audio frame
  return processAudioFrame(gist->audioFrame);
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
