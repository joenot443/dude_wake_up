////
////  AudioStream.cpp
////  dude_wake_up
////
////  Created by Joe Crozier on 2022-05-30.
////
//
//#include "AudioStream.hpp"
////#include "BTrack.h"
//#include "Console.hpp"
//
//void AudioStream::setup() {
//  ofSoundDevice device = ofSoundStream().getDeviceList()[config.index];
//  
//  
//  ofSoundStreamSettings settings = ofSoundStreamSettings();
//  settings.setInDevice(device);
////  settings.bufferSize = 512;
//  settings.sampleRate = 44100;
//  settings.numInputChannels = 1;
//  settings.numOutputChannels = 0;
//  settings.inCallback = [this](ofSoundBuffer &soundBuffer) {
//    this->audioIn(soundBuffer);
//  };
//  
//  stream.setup(settings);
//  analysis->beatOscillator->enabled = true;
//}
//
//void AudioStream::update() {
////  analysis->analyzeFrame(&gist);
////  cout<<"============"<<endl;
////  cout<<"rms: \t" << gist.rootMeanSquare() << std::endl;
////  cout<<"pitch: \t" << gist.pitch() << std::endl;
////  cout<<"csd: \t" << gist.complexSpectralDifference() << std::endl;
////  cout<<"zcr: \t" << gist.zeroCrossingRate() << std::endl;
////  cout<<"specDiff: \t" << gist.spectralDifference() << std::endl;
////  cout<<"mod_rms: \t" << analysis->rms.value << std::endl;
////  cout<<"mod_pitch: \t" << analysis->pitch.value << std::endl;
////  cout<<"mod_csd: \t" << analysis->csd.value << std::endl;
////  cout<<"mod_zcr: \t" << analysis->zcr.value << std::endl;
////  cout<<"mod_specDiff: \t" << analysis->spectralDiff.value << std::endl;
////  cout<<"============"<<endl;
//}
//
//void AudioStream::audioIn(ofSoundBuffer &soundBuffer) {
//  float avg = 0.0;
//  for (int i = 0; i < soundBuffer.size(); i++){
//      float leftSample = soundBuffer[i * 2] * 0.5;
//      float rightSample = soundBuffer[i * 2 + 1] * 0.5;
//    avg += leftSample + rightSample;
//  }
//  avg = avg / soundBuffer.size();
//  
//  // If we have mostly silence, return
//  if (avg < 0.01) {
//    return;
//  }
//  
//  auto buffer = soundBuffer.getBuffer();
//  auto doubleBuffer = std::vector<double>(buffer.begin(), buffer.end());
//  double* firstDouble = doubleBuffer.data();
////  gist.processAudioFrame(buffer);
////  b.processAudioFrame(firstDouble);
//  
////  if (b.beatDueInCurrentFrame()) {
////    analysis->beatOscillator->pulse();
////  }
//}
