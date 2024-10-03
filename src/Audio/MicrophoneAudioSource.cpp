//
//  MicrophoneAudioSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/21/24.
//

#include "MicrophoneAudioSource.hpp"

void MicrophoneAudioSource::setup() {
    ofSoundStreamSettings settings;
    settings.setInDevice(device);
    settings.sampleRate = 44100;
    settings.numInputChannels = 1;
    settings.numOutputChannels = 0;
    settings.bufferSize = 512;
    settings.inCallback = [this](ofSoundBuffer &soundBuffer) {
        this->audioIn(soundBuffer);
    };
    gist.setAudioFrameSize(512);
    gist.setSamplingFrequency(44100);
    active = true;
    stream.setup(settings);
}

void MicrophoneAudioSource::toggle() {
    AudioSource::toggle();
}

void MicrophoneAudioSource::disable() {
    active = false;
    stream.stop();
    stream.close();
}

void MicrophoneAudioSource::audioIn(ofSoundBuffer &soundBuffer) {
    AudioSource::audioIn(soundBuffer);
}
