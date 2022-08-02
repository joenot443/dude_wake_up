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
  
  ImGui::Columns(2, name.c_str(), false);  // 3-ways, no border
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
  CommonViews::Spacing(8);
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Feedback Parameters");
  ImGui::Spacing();
  ImGui::PopFont();
  
  ImGui::Checkbox("Enabled##fb0_enabled", &feedbackSettings->enabled);
  
  // Mix
  CommonViews::SliderWithOscillator("Mix", "##mix_amount", &feedbackSettings->mixSettings.mix, &feedbackSettings->mixSettings.mixOscillator);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.mix);
  
  // Delay Amount
  CommonViews::SliderWithOscillator("Delay", "##delay_amount", &feedbackSettings->mixSettings.delayAmount, &feedbackSettings->mixSettings.delayAmountOscillator);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.delayAmount);
  
  // Key Value
  CommonViews::SliderWithOscillator("Key Value", "##key_value", &feedbackSettings->mixSettings.keyValue, &feedbackSettings->mixSettings.keyValueOscillator);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.keyValue);
  
  // Threshold
  CommonViews::SliderWithOscillator("Key Threshold", "##key_threshold", &feedbackSettings->mixSettings.keyThreshold, &feedbackSettings->mixSettings.keyThresholdOscillator);
  CommonViews::MidiSelector(&feedbackSettings->mixSettings.keyThreshold);
  
  CommonViews::Spacing(8);
}

void FeedbackSettingsView::drawHSB() {
  CommonViews::Spacing(8);
  
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Basic (HSB)");
  ImGui::Spacing();
  ImGui::PopFont();
  
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
  
  CommonViews::Spacing(8);
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
  
  // Scale x
  ImGui::Text("Scale x");
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("##scalex", &feedbackSettings->scaleSettings.xScale.value, 0.0001, 10.0, "%.3f");
  ImGui::PopItemWidth();
  CommonViews::MidiSelector(&feedbackSettings->scaleSettings.xScale);
  
  
  // Scale y
  ImGui::Text("Scale y");
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("##scaley", &feedbackSettings->scaleSettings.yScale.value, 0.0001, 10.0, "%.3f");
  ImGui::PopItemWidth();
  CommonViews::MidiSelector(&feedbackSettings->scaleSettings.yScale);
  
  // Mirror Horizontal
  ImGui::Text("Mirror Horizontally");
  ImGui::Checkbox("", &feedbackSettings->miscSettings.horizontalMirror);
  
  // Mirror Vertical
  ImGui::Text("Mirror Vertically");
  ImGui::Checkbox("", &feedbackSettings->miscSettings.verticalMirror);
}
