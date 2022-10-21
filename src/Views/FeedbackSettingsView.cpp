//
//  FeedbackSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-13.
//

#include "FeedbackSettingsView.hpp"
#include "CommonViews.hpp"
#include "FontService.hpp"
#include "OscillatorView.hpp"


void FeedbackSettingsView::draw() {
  auto name = std::string("##%dfeedbacksettings", feedbackSettings->index);
  
  ImGui::Columns(2, name.c_str(), false);
  drawHSB();
  ImGui::NextColumn();
  drawParameters();
  ImGui::NextColumn();
  drawRanges();
  ImGui::NextColumn();
  drawMisc();
  ImGui::Columns(1);
}

void FeedbackSettingsView::drawParameters() {
  CommonViews::H3Title("Feedback Parameters");
  
  ImGui::Checkbox("Use Processed##fb_use_processed", &feedbackSettings->useProcessedFrame.boolValue);
  
  ImGui::Checkbox("Luma Key Enabled##fb_luma_key", &feedbackSettings->lumaKeyEnabled.boolValue);
  
  // Mix
  CommonViews::SliderWithOscillator("Blend", "##blend_amount", &feedbackSettings->mixSettings.blend, &feedbackSettings->mixSettings.blendOscillator);
  CommonViews::ModulationSelector(&feedbackSettings->mixSettings.blend);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.blend);
  
  // Delay Amount
  CommonViews::IntSliderWithOscillator("Delay", "##delay_amount", &feedbackSettings->mixSettings.delayAmount, &feedbackSettings->mixSettings.delayAmountOscillator);
  CommonViews::ModulationSelector(&feedbackSettings->mixSettings.delayAmount);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.delayAmount);

  if (feedbackSettings->lumaKeyEnabled.boolValue) {
    // Key Value
    CommonViews::SliderWithOscillator("Key Value", "##key_value", &feedbackSettings->mixSettings.keyValue, &feedbackSettings->mixSettings.keyValueOscillator);
    CommonViews::ModulationSelector(&feedbackSettings->mixSettings.keyValue);
    CommonViews::MidiSelector(&feedbackSettings->mixSettings.keyValue);
    
    // Threshold
    CommonViews::SliderWithOscillator("Key Threshold", "##key_threshold", &feedbackSettings->mixSettings.keyThreshold, &feedbackSettings->mixSettings.keyThresholdOscillator);
    CommonViews::ModulationSelector(&feedbackSettings->mixSettings.keyThreshold);
    CommonViews::MidiSelector(&feedbackSettings->mixSettings.keyThreshold);
  }
}

void FeedbackSettingsView::drawHSB() {
  CommonViews::H3Title("Feedback HSB");
  
  // Hue
  CommonViews::SliderWithInvertOscillator("Hue", "##hue", &feedbackSettings->hsbSettings.hue, &feedbackSettings->hsbSettings.invertHue, &feedbackSettings->hsbSettings.hueOscillator);
  CommonViews::ModulationSelector(&feedbackSettings->hsbSettings.hue);
  CommonViews::MidiSelector(&feedbackSettings->hsbSettings.hue);
  
  // Saturation
  CommonViews::SliderWithInvertOscillator("Saturation", "##saturation", &feedbackSettings->hsbSettings.saturation, &feedbackSettings->hsbSettings.invertSaturation, &feedbackSettings->hsbSettings.saturationOscillator);
  CommonViews::ModulationSelector(&feedbackSettings->hsbSettings.saturation);
  CommonViews::MidiSelector(&feedbackSettings->hsbSettings.saturation);
  
  // Brightness
  CommonViews::SliderWithInvertOscillator("Brightness", "##brightness", &feedbackSettings->hsbSettings.brightness, &feedbackSettings->hsbSettings.invertBrightness, &feedbackSettings->hsbSettings.brightnessOscillator);
  CommonViews::ModulationSelector(&feedbackSettings->hsbSettings.brightness);
  CommonViews::MidiSelector(&feedbackSettings->hsbSettings.brightness);
}

void FeedbackSettingsView::drawRanges() {
  CommonViews::Spacing(8);
  
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Displace");
  ImGui::Spacing();
  ImGui::PopFont();
  
  // Displace x
  
  CommonViews::SliderWithOscillator("Displace x",
                                    "##displace_x",
                                    &feedbackSettings->miscSettings.xOffset,
                                    &feedbackSettings->miscSettings.xOffsetOscillator);
  CommonViews::MidiSelector(&feedbackSettings->miscSettings.xOffset);
  
  // Displace y
  CommonViews::SliderWithOscillator("Displace y",
                                    "##displace_y",
                                    &feedbackSettings->miscSettings.yOffset,
                                    &feedbackSettings->miscSettings.yOffsetOscillator);
  CommonViews::MidiSelector(&feedbackSettings->miscSettings.yOffset);
  
  // Rotate
  CommonViews::SliderWithOscillator("Rotate",
                                    "##rotate",
                                    &feedbackSettings->miscSettings.rotate,
                                    &feedbackSettings->miscSettings.rotationOscillator);
  CommonViews::MidiSelector(&feedbackSettings->miscSettings.rotate);
  
  CommonViews::Spacing(8);
}

void FeedbackSettingsView::drawMisc() {
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Misc");
  ImGui::Spacing();
  ImGui::PopFont();
  
  // Scale
  CommonViews::SliderWithOscillator("Scale",
                                    "##fb_scale",
                                    &feedbackSettings->miscSettings.scale,
                                    &feedbackSettings->miscSettings.scaleOscillator);
  CommonViews::MidiSelector(&feedbackSettings->miscSettings.scale);

  // Mirror Horizontal
  ImGui::Text("Mirror Horizontally");
  ImGui::Checkbox("##mirror_horizontal", &feedbackSettings->miscSettings.horizontalMirror);
  
  // Mirror Vertical
  ImGui::Text("Mirror Vertically");
  ImGui::Checkbox("##mirror_vertical", &feedbackSettings->miscSettings.verticalMirror);
}
