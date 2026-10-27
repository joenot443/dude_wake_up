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
    // Pre-configure Gist before the audio callback can fire (a failed
    // stream.setup won't fire, but concurrent processFrame from a stale
    // running stream would race an un-sized Gist otherwise).
    gist.setAudioFrameSize(512);
    gist.setSamplingFrequency(44100);

    // Initialize bpmStartTime to current time
    auto currentTime = std::chrono::steady_clock::now();
    audioAnalysis.bpmStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    // active reflects whether the stream is really open. Setting it before
    // stream.setup() would lie to consumers if the backend rejects our
    // requested format (Windows WASAPI shared mode is picky — a mic that
    // only exposes 48 kHz will fail a 44.1 kHz open silently on some
    // driver stacks). setup() returns bool in OF 0.11+; treat false as
    // failure and leave active=false so the UI can surface it.
    active = stream.setup(settings);
    if (!active) {
        ofLogWarning("MicrophoneAudioSource")
          << "device " << device.deviceID
          << " (\"" << device.name << "\") rejected 44.1 kHz mono @ 512 frames"
          << " — leaving source inactive";
    }
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
