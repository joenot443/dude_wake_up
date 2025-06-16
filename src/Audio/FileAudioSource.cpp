#include "FileAudioSource.hpp"
#include "AudioSourceService.hpp"
#include "ofLog.h"

FileAudioSource::~FileAudioSource() {
  disable();
}

void FileAudioSource::setup() {
  if (track == nullptr) {
    track = AudioSourceService::getService()->defaultSampleAudioTrack();
  }
  
  audioFile.setVerbose(true); // Enable verbose logging if needed
  audioFile.load(track->path);
  
  if (audioFile.loaded()) {
    // Initialize Gist with the correct frame size and sample rate
    gist.setAudioFrameSize(frameSize);
    gist.setSamplingFrequency(audioFile.samplerate());
    
    // Configure the sound stream
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = audioFile.samplerate();
    settings.numInputChannels = 0;
    settings.numOutputChannels = audioFile.channels();
    settings.bufferSize = frameSize;
    
    soundStream.setup(settings);
    
    active = true;
    currentSample = 0;
    isPaused = false;
  } else {
    ofLogError() << "Failed to load audio file: " << track->path;
    active = false;
  }
}

void FileAudioSource::loadFile(std::shared_ptr<AudioTrack> audioTrack) {
  track = audioTrack;
  disable();
  setup();
  active = true;
  // Start the BPM clock
  audioAnalysis.bpmEnabled = true;
  audioAnalysis.bpmStartTime = ofGetSystemTimeMillis();
}

void FileAudioSource::disable() {
  active = false;
  
  soundStream.stop();
  soundStream.close();
  
  currentSample = 0;
  isPaused = false;
}

void FileAudioSource::pausePlayback() {
  isPaused.store(true);
  audioAnalysis.bpmEnabled = false;
}

void FileAudioSource::resumePlayback() {
  isPaused.store(false);
  audioAnalysis.bpmEnabled = true;
}

void FileAudioSource::setVolume(float vol) {
  std::lock_guard<std::mutex> lock(audioMutex);
  volume = vol;
}

void FileAudioSource::audioOut(ofSoundBuffer &outputBuffer) {
  std::lock_guard<std::mutex> lock(audioMutex);
  
  if (!active) {
    outputBuffer.set(0.0f); // Silence the output
    return;
  }
  
  if (isPaused.load()) {
    outputBuffer.set(0.0f); // Silence the output
    return; // Do not advance currentSample
  }
  
  uint64_t totalSamplesPerChannel = audioFile.length(); // Total samples per channel
  unsigned int numChannels = audioFile.channels();
  unsigned int outputChannels = outputBuffer.getNumChannels();
  size_t bufferNumFrames = outputBuffer.getNumFrames();
  
  // Prepare a mono buffer for analysis
  ofSoundBuffer analysisBuffer;
  analysisBuffer.allocate(bufferNumFrames, 1); // Mono buffer
  analysisBuffer.setSampleRate(audioFile.samplerate());
  
  // Get references to the buffers
  auto& outputBufferData = outputBuffer.getBuffer();
  auto& analysisBufferData = analysisBuffer.getBuffer();
  
  for (size_t i = 0; i < bufferNumFrames; ++i) {
    uint64_t sampleIndex = currentSample + i;
    
    // Implement looping
    if (sampleIndex >= totalSamplesPerChannel) {
      if (loop) {
        sampleIndex = sampleIndex % totalSamplesPerChannel;
      } else {
        // Fill the rest of the buffer with silence and stop playback
        for (; i < bufferNumFrames; ++i) {
          for (unsigned int c = 0; c < outputChannels; ++c) {
            size_t index = i * outputChannels + c;
            outputBufferData[index] = 0.0f;
          }
          analysisBufferData[i] = 0.0f;
        }
        active = false;
        break;
      }
    }
    
    float sampleSum = 0.0f;
    // Sum samples from all channels
    for (unsigned int c = 0; c < numChannels; ++c) {
      float sample = audioFile.sample(sampleIndex, c);
      sampleSum += sample;
    }
    float averagedSample = sampleSum / numChannels;
    
    // Set the averaged sample to each output channel in the output buffer, applying volume
    for (unsigned int c = 0; c < outputChannels; ++c) {
      size_t index = i * outputChannels + c;
      outputBufferData[index] = averagedSample * volume; // Apply volume here
    }
    
    // Store the original averaged sample in the analysis buffer (unaffected by volume)
    analysisBufferData[i] = averagedSample;
  }
  
  // Process the buffer for analysis
  audioIn(analysisBuffer);
  
  // Advance the current sample index
  currentSample += bufferNumFrames;
  
  if (loop) {
    currentSample = currentSample % totalSamplesPerChannel;
  } else if (currentSample >= totalSamplesPerChannel) {
    currentSample = totalSamplesPerChannel; // Ensure we don't exceed total samples
  }
}

// Playback position methods remain unchanged...

float FileAudioSource::getCurrentPlaybackTime() const {
  std::lock_guard<std::mutex> lock(audioMutex);
  uint64_t samplePosition = currentSample % audioFile.length();
  return static_cast<float>(samplePosition) / audioFile.samplerate();
}

float FileAudioSource::getTotalDuration() const {
  return static_cast<float>(audioFile.length()) / audioFile.samplerate();
}

float FileAudioSource::getPlaybackPosition() const {
  std::lock_guard<std::mutex> lock(audioMutex);
  uint64_t totalSamples = audioFile.length();
  if (totalSamples == 0) return 0.0f; // Avoid division by zero
  uint64_t samplePosition = currentSample % totalSamples;
  return static_cast<float>(samplePosition) / totalSamples;
}

void FileAudioSource::setPlaybackPosition(float position) {
  if (position < 0.0f) position = 0.0f;
  if (position > 1.0f) position = 1.0f;
  uint64_t totalSamples = audioFile.length();
  {
    std::lock_guard<std::mutex> lock(audioMutex);
    currentSample = static_cast<uint64_t>(position * totalSamples);
  }
}
