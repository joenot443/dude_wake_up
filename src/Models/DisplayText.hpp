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
#include "Settings.hpp"
#include "UUID.hpp"
#include "Strings.hpp"

struct Font {
  std::string name = "";
  
  std::string path() {
    return formatString("fonts/editor/%s.ttf", name.c_str());
  }
  
  Font(std::string name) : name(name) {};
};


class DisplayText: public Settings {
public:
  std::string text = "Sample text";
  std::string id;
  ofColor color = ofColor::white;
  int fontSize = 36;
  Font font = Font("");
  
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  
  std::shared_ptr<Oscillator> xPositionOscillator;
  std::shared_ptr<Oscillator> yPositionOscillator;
  
  DisplayText() : font(Font("")),
  id(UUID::generateUUID()),
  xPosition(std::make_shared<Parameter>("xPosition", 0.1, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("yPosition", 0.1, 0.0, 1.0)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition))
  {
    oscillators = {xPositionOscillator, yPositionOscillator};
    parameters = {xPosition, yPosition};
    registerParameters();
  };
};

#endif /* DisplayText_h */
