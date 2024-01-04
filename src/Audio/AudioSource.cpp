//
//  AudioSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#include "AudioSource.hpp"
#include "Console.hpp"
#include "AudioSourceService.hpp"
#include "Gist.h"
#include "BTrack.h"
#include <stdio.h>

void AudioSource::setup()
{
  ofSoundStreamSettings settings = ofSoundStreamSettings();
  settings.setInDevice(device);
  settings.sampleRate = 44100;
  settings.numInputChannels = 1;
  settings.numOutputChannels = 0;
  settings.bufferSize = 512;
  settings.inCallback = [this](ofSoundBuffer &soundBuffer)
  {
    this->audioIn(soundBuffer);
  };
  gist.setAudioFrameSize(512);
  gist.setSamplingFrequency(44100);
  active = true;
  stream.setup(settings);
}

void AudioSource::update() {}

void AudioSource::toggle()
{
  if (active)
    disable();
  else
    setup();
}

void AudioSource::disable()
{
  active = false;
  stream.stop();
  stream.close();
}

void AudioSource::audioIn(ofSoundBuffer &soundBuffer)
{
  float avg = 0.0;
  for (int i = 0; i < soundBuffer.size() / 2; i++)
  {
    float leftSample = soundBuffer[i * 2] * 0.5;
    float rightSample = soundBuffer[i * 2 + 1] * 0.5;
    avg += abs(leftSample) + abs(rightSample);
  }
  avg = avg / static_cast<float>(soundBuffer.size());
  
  // If we have mostly silence, return
  if (abs(avg) < 0.01)
  {
    return;
  }
  
  auto buffer = soundBuffer.getBuffer();
  processFrame(buffer);
  debugGist();
}

void AudioSource::processFrame(std::vector<float> frame)
{
  gist.processAudioFrame(frame);
  audioAnalysis.analyzeFrame(&gist);
  
//  // Convert to a double* for bTrack
//  double *doubleFrame = new double[frame.size()];
//  for (int i = 0; i < frame.size(); i++)
//  {
//    doubleFrame[i] = frame[i];
//  }
//  
//  
//  
//  bTrack.processAudioFrame(doubleFrame);
//  if (bTrack.beatDueInCurrentFrame()) {
//    log("That's a beat!");
//  }
}

void AudioSource::debugGist()
{
  //    cout<<"============"<<endl;
  //    cout<<"rms: \t" << gist.rootMeanSquare() << std::endl;
  //    cout<<"pitch: \t" << gist.pitch() << std::endl;
  //    cout<<"csd: \t" << gist.complexSpectralDifference() << std::endl;
  //    cout<<"zcr: \t" << gist.zeroCrossingRate() << std::endl;
  //    cout<<"specDiff: \t" << gist.spectralDifference() << std::endl;
  //    cout<<"============"<<endl;
}
