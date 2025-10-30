//
//  TextSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#ifndef TextSource_hpp
#define TextSource_hpp

#include "ofMain.h"
#include "VideoSource.hpp"
#include "TextEditorView.hpp"
#include "DisplayText.hpp"

class TextSource : public VideoSource {
public:
  TextSource(std::string id, std::string name, std::shared_ptr<DisplayText> displayText)
  : VideoSource(id, name, VideoSource_text), displayText(displayText), textEditorView(TextEditorView(displayText)) {};
  
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;

protected:
  TextEditorView textEditorView;
  ofTrueTypeFont font;
  std::string fontPath;
  std::shared_ptr<DisplayText> displayText;
  std::string lastText;
  ofShader strokeShader;
  ofFbo tempFbo;
  float xPos = 0.0;
  float yPos = 0.0;
};

#endif /* TextSource_hpp */
