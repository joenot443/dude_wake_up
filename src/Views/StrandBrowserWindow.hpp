//
//  StrandBrowserWindow.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/30/24.
//

#ifndef StrandBrowserWindow_hpp
#define StrandBrowserWindow_hpp

#include <stdio.h>
#include "StrandService.hpp"

enum StrandBrowserType {
  StrandBrowserType_Recent,
  StrandBrowserType_Template
};

class StrandBrowserWindow {
public:
  StrandBrowserType type;
  std::vector<std::shared_ptr<AvailableStrand>> strands;

  StrandBrowserWindow(StrandBrowserType type) : type(type) {
    
  };
  void setup();
  void draw();
  void update();
};


#endif /* StrandBrowserWindow_hpp */
