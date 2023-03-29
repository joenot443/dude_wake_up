//
//  DisplayText.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#ifndef DisplayText_h
#define DisplayText_h
#include "Parameter.hpp"

struct Font {
  std::string path = "";
  std::string name = "";
  Font(std::string name, std::string path) : path(path), name(name) {};
};


struct DisplayText {
  std::string text = "Sample text";
  ofColor color = ofColor::white;
  int fontSize = 12;
  Font font = Font("", "");
  
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
public:
  DisplayText() : font(Font("","")),
  xPosition(std::make_shared<Parameter>("xPosition", "xPosition", 0.1, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("yPosition", "xPosition", 0.1, 0.0, 1.0))
  {};
};

#endif /* DisplayText_h */
