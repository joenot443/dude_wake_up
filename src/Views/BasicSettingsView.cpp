//
//  BasicSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "BasicSettingsView.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"


void BasicSettingsView::setup() {
  
}

void BasicSettingsView::update() {
}

void BasicSettingsView::draw() {
  if (basicSettings == NULL) {
    return;
  }
  ImGui::Columns(2, "videofeedsettings", false);  // 3-ways, no border
  drawHSB();
  drawPixelation();
  ImGui::NextColumn();
  drawBlurSharpen();
  drawTransform();
  ImGui::Columns(1);
  CommonViews::Spacing(16);
}

void BasicSettingsView::drawHSB() {
  CommonViews::H3Title("Basic (HSB)");
  
  // Hue
  CommonViews::SliderWithInvertOscillator("Hue", "##hue", &basicSettings->hsbSettings.hue, &basicSettings->hsbSettings.invertHue, &basicSettings->hsbSettings.hueOscillator);
  CommonViews::ModulationSelector(&basicSettings->hsbSettings.hue);
  CommonViews::MidiSelector(&basicSettings->hsbSettings.hue);
  
  // Saturation
  CommonViews::SliderWithInvertOscillator("Saturation", "##saturation", &basicSettings->hsbSettings.saturation, &basicSettings->hsbSettings.invertSaturation, &basicSettings->hsbSettings.saturationOscillator);
  CommonViews::ModulationSelector(&basicSettings->hsbSettings.saturation);
  CommonViews::MidiSelector(&basicSettings->hsbSettings.saturation);

  // Brightness
  CommonViews::SliderWithInvertOscillator("Brightness", "##brightness", &basicSettings->hsbSettings.brightness, &basicSettings->hsbSettings.invertBrightness, &basicSettings->hsbSettings.brightnessOscillator);
  CommonViews::ModulationSelector(&basicSettings->hsbSettings.brightness);
  CommonViews::MidiSelector(&basicSettings->hsbSettings.brightness);
  
  // Amount
  CommonViews::SliderWithOscillator("Scale", "##scale", &basicSettings->transformSettings.scale, &basicSettings->transformSettings.scaleOscillator);
  CommonViews::ModulationSelector(&basicSettings->transformSettings.scale);
  CommonViews::MidiSelector(&basicSettings->transformSettings.scale);
}

void BasicSettingsView::drawBlurSharpen() {
  CommonViews::H3Title("Blur");
  
  // Amount
  CommonViews::SliderWithOscillator("Amount", "##amount", &basicSettings->blurSettings.amount, &basicSettings->blurSettings.amountOscillator);
  CommonViews::ModulationSelector(&basicSettings->blurSettings.amount);
  CommonViews::MidiSelector(&basicSettings->blurSettings.amount);
  
  // Radius
  CommonViews::SliderWithOscillator("Radius", "##radius", &basicSettings->blurSettings.radius, &basicSettings->blurSettings.radiusOscillator);
  CommonViews::ModulationSelector(&basicSettings->blurSettings.radius);
  CommonViews::MidiSelector(&basicSettings->blurSettings.radius);

  // Sharpen
  
  CommonViews::H3Title("Sharpen");
  
  CommonViews::SliderWithOscillator("Amount", "##sharpen_amount", &basicSettings->sharpenSettings.amount, &basicSettings->sharpenSettings.amountOscillator);
  CommonViews::ModulationSelector(&basicSettings->sharpenSettings.amount);
  CommonViews::MidiSelector(&basicSettings->sharpenSettings.amount);

  // Radius
  CommonViews::SliderWithOscillator("Radius", "##sharpen_radius", &basicSettings->sharpenSettings.radius, &basicSettings->sharpenSettings.radiusOscillator);
  CommonViews::ModulationSelector(&basicSettings->sharpenSettings.radius);
  CommonViews::MidiSelector(&basicSettings->sharpenSettings.radius);

  // Boost
  CommonViews::SliderWithOscillator("Boost", "##sharpen_boost", &basicSettings->sharpenSettings.boost, &basicSettings->sharpenSettings.boostOscillator);
  CommonViews::ModulationSelector(&basicSettings->sharpenSettings.boost);
  CommonViews::MidiSelector(&basicSettings->sharpenSettings.boost);
}

void BasicSettingsView::drawTransform() {
  
}

void BasicSettingsView::drawPixelation() {  
  CommonViews::H3Title("Pixelation");

  ImGui::Text("Mix");
  ImGui::SetNextItemWidth(150.0);
  ImGui::SliderFloat("##pixelation_mix", &basicSettings->pixelSettings.mix.value, basicSettings->pixelSettings.mix.min, basicSettings->pixelSettings.mix.max, "%.3f");
  ImGui::SameLine(0, 20);
  ImGui::Checkbox("Enabled##pixelation_enabled", &basicSettings->pixelSettings.enabled);
  CommonViews::ModulationSelector(&basicSettings->pixelSettings.mix);
  CommonViews::MidiSelector(&basicSettings->pixelSettings.mix);

  CommonViews::SliderWithOscillator("Scale", "##pixelation_scale", &basicSettings->pixelSettings.scale, &basicSettings->pixelSettings.scaleOscillator);
  CommonViews::ModulationSelector(&basicSettings->pixelSettings.scale);
  CommonViews::MidiSelector(&basicSettings->pixelSettings.scale);
}

