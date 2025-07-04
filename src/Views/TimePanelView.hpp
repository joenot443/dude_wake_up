#ifndef TimePanelView_hpp
#define TimePanelView_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <tuple>
#include "Parameter.hpp"
#include "AudioSettings.hpp"

class TimePanelView {
public:
  void setup();
  void update();
  void draw();
};

#endif /* TimePanelView_hpp */