//
//  AudioSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#include "AudioSettingsView.hpp"
#include "OscillatorView.hpp"

void AudioSettingsView::setup() {
  
}

void AudioSettingsView::draw() {
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;
  if (ImGui::Begin("audio_settings_view", NULL, windowFlags)) {
    ImGui::Text("Test");
    ImGui::SliderFloat("Pulse", &analysis->beat->value, 0.0, 1.0);
//    OscillatorView::draw("Pulse", &analysis->beat, &analysis->beatOscillator);
//    OscillatorView::draw("RMS", &analysis->rms, &analysis->rmsOscillator);
//    OscillatorView::draw("Pitch", &analysis->pitch, &analysis->pitchOscillator);
//    OscillatorView::draw("CSD", &analysis->csd, &analysis->csdOscillator);
//    OscillatorView::draw("Energy", &analysis->energy, &analysis->energyOscillator);
//    OscillatorView::draw("ZCR", &analysis->zcr, &analysis->zcrOscillator);
  }
  ImGui::End();
}

void AudioSettingsView::update() {
  
}
