//
//  FontService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-10.
//

#ifndef FontService_hpp
#define FontService_hpp

#include <stdio.h>
// load the fonts here and then set them in the TextSource init directly
#include "Font.hpp"
#include "ofxImGui.h"



class FontService
{
  static FontService* service;
 
  FontService() {
    
  }
public:
  struct FontSet {
    ImFontAtlas* atlas = nullptr;
    GLuint texID = 0;
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
    ImFont *pN;
    ImGuiStyle style;
  };
  
  // Fonts
  FontSet retinaFonts;
  FontSet standardFonts;
  FontSet *current;
  
  ImVec2 paddingH1 = ImVec2(0, 30);
  ImVec2 paddingH2 = ImVec2(0, 8);
  ImVec2 paddingH3 = ImVec2(0, 10);
  ImVec2 paddingH4 = ImVec2(0, 10);

  // Redesign
  std::vector<Font> fonts;
  std::vector<std::string> fontNames;
  
  void loadFonts();
  void setup();

  void loadFontSet(FontSet& set, float scale);
  void useFontSetForScale(float scale);

  // Get font path by name
  std::string fontPathByName(std::string name);
  
  static FontService* getService() {
    if (!service) {
      service = new FontService;
    }
    return service;
  }
};



#endif /* FontService_hpp */
