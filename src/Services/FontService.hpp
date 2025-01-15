//
//  FontService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#ifndef FontService_hpp
#define FontService_hpp

#include <stdio.h>
#include "ofxImGui.h"

class FontService
{
  static FontService* service;
 
  FontService() {
    
  }
public:
  // Fonts
  ImFont * h1;
  ImFont * h2;
  ImFont * h3;
  ImFont * h3b;
  ImFont * h4;
  ImFont * audio;
  ImFont * sm;
  ImFont * p;
  ImFont * b;
  ImFont * i;
  ImFont * icon;
  ImFont * largeIcon;
  ImFont * xLargeIcon;
  
  ImVec2 paddingH1 = ImVec2(0, 30);
  ImVec2 paddingH2 = ImVec2(0, 12);
  ImVec2 paddingH3 = ImVec2(0, 10);
  ImVec2 paddingH4 = ImVec2(0, 10);

  // Redesign
  
  ImFont *pN;
  
  void loadFonts();
  void addFontToGui(ofxImGui::Gui *gui);
  
  static FontService* getService() {
    if (!service) {
      service = new FontService;
    }
    return service;
  }
};


#endif /* FontService_hpp */
