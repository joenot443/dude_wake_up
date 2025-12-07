//
//  OscillatorPanelView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/22/24.
//

#ifndef OscillatorPanelView_hpp
#define OscillatorPanelView_hpp

#include <stdio.h>
#include <memory>

class Oscillator;

struct OscillatorPanelView {
public:
  void setup();
  void update();
  void draw();
  void drawOscillatorSettings(std::shared_ptr<Oscillator> oscillator);

  OscillatorPanelView() {

  }
};

#endif /* OscillatorPanelView_hpp */
