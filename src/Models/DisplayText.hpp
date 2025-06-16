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

class DisplayText: public Settings {
public:
  std::string text = "Sample text";
  std::string id;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> strokeColor;
  std::shared_ptr<Parameter> strokeWeight;
  std::shared_ptr<Parameter> strokeEnabled;

  int fontSize = 36;
  Font font = FontService::getService()->fonts[0];
    
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  
  std::shared_ptr<Oscillator> xPositionOscillator;
  std::shared_ptr<Oscillator> yPositionOscillator;
  
  std::shared_ptr<Parameter> fontSelector;
  
  DisplayText() : font(Font("", "")),
  id(UUID::generateUUID()),
  xPosition(std::make_shared<Parameter>("X", 0.1, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y", 0.1, 0.0, 1.0)),
  strokeWeight(std::make_shared<Parameter>("Stroke Weight", 5.0, 0.0, 20.0)),
  color(std::make_shared<Parameter>("Color", ParameterType_Color)),
  strokeColor(std::make_shared<Parameter>("Stroke Color", ParameterType_Color)),
  strokeEnabled(std::make_shared<Parameter>("Stroke", ParameterType_Bool)),
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
    
    color->setColor({1.0, 1.0, 1.0, 1.0});
    fontSelector->options = fontNames;
    oscillators = {xPositionOscillator, yPositionOscillator};
    parameters = {xPosition, yPosition, strokeEnabled, fontSelector};
    registerParameters();
  };
};

#endif /* DisplayText_h */
