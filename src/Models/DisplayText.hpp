//
//  DisplayText.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#ifndef DisplayText_h
#define DisplayText_h
#include "Parameter.hpp"
#include "WaveformOscillator.hpp"
#include "ConfigService.hpp"
#include "FontService.hpp"
#include "Settings.hpp"
#include "Font.hpp"
#include "UUID.hpp"
#include "Strings.hpp"
#include <algorithm>

class DisplayText: public Settings {
public:
  std::string text = "Sample text";
  std::string id;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> strokeColor;
  std::shared_ptr<Parameter> strokeWeight;
  std::shared_ptr<Parameter> strokeEnabled;
  std::shared_ptr<Parameter> edgeSoftness;
  std::shared_ptr<Parameter> textSmoothing;

  int fontSize = 72;
  Font font = FontService::getService()->fonts[0];
    
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  
  std::shared_ptr<Oscillator> xPositionOscillator;
  std::shared_ptr<Oscillator> yPositionOscillator;
  
  std::shared_ptr<Parameter> fontSelector;
  
  DisplayText() : font(Font("", "")),
  id(UUID::generateUUID()),
  xPosition(std::make_shared<Parameter>("X", 0.1, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y", 0.4, 0.0, 1.0)),
  strokeWeight(std::make_shared<Parameter>("Stroke Weight", 5.0, 0.0, 20.0)),
  color(std::make_shared<Parameter>("Color", ParameterType_Color)),
  strokeColor(std::make_shared<Parameter>("Stroke Color", ParameterType_Color)),
  strokeEnabled(std::make_shared<Parameter>("Stroke", ParameterType_Bool)),
  edgeSoftness(std::make_shared<Parameter>("Edge Smoothing", 0.8, 0.0, 2.0)),
  textSmoothing(std::make_shared<Parameter>("Text Antialiasing", 8.0, 0.0, 16.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  fontSelector(std::make_shared<Parameter>("Font", 0.0, 0.0, 0.0, ParameterType_Int))
  {
    // Initialize font parameter if not already set
    std::vector<std::string> fontNames;
    ofDirectory fontsDir = ofDirectory("fonts/editor");
    fontsDir.listDir();

    // Collect font names
    for (auto& file : fontsDir.getFiles()) {
      fontNames.push_back(removeFileExtension(file.getFileName()));
    }

    // Sort font names alphabetically to match FontService
    std::sort(fontNames.begin(), fontNames.end());

    // Find Roboto index
    int robotoIndex = 0;
    for (int i = 0; i < fontNames.size(); i++) {
      if (fontNames[i] == "Roboto") {
        robotoIndex = i;
        break;
      }
    }

    color->setColor({1.0, 1.0, 1.0, 1.0});
    fontSelector->options = fontNames;
    fontSelector->intValue = robotoIndex;
    fontSelector->value = static_cast<float>(robotoIndex);

    // Set default font to Roboto if available
    if (robotoIndex < FontService::getService()->fonts.size()) {
      font = FontService::getService()->fonts[robotoIndex];
    }

    oscillators = {xPositionOscillator, yPositionOscillator};
    parameters = {xPosition, yPosition, strokeEnabled, fontSelector, edgeSoftness, textSmoothing};
    registerParameters();
  };
};

#endif /* DisplayText_h */
