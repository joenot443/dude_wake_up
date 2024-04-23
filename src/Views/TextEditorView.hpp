//
//  TextEditorView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#ifndef TextEditorView_hpp
#define TextEditorView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxImGui.h"
#include "DisplayText.hpp"

class TextEditorView {
public:
  TextEditorView(std::shared_ptr<DisplayText> displayText);
  
  void draw();
  void drawPositionSlider();
private:
  std::shared_ptr<DisplayText> displayText;
  ofTrueTypeFont font;
  std::vector<Font> fonts;
  std::string id;
  int selectedFontIndex;
};

#endif /* TextEditorView_hpp */
