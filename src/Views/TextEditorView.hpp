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
#include "ofFbo.h"
#include "FontService.hpp"
#include "ofShader.h"
#include <map>
#include <string>
#include "ImGuiExtensions.hpp"

class TextEditorView {
public:
  TextEditorView(std::shared_ptr<DisplayText> displayText);
  
  void draw();
  void drawPositionSlider();

private:
  std::shared_ptr<DisplayText> displayText;
  ofTrueTypeFont font;
  std::string id;
  std::string lastPreviewText = "";
  bool previewsNeedUpdate = true;
  int frameCount = 0;

  // Font Preview Members
  std::map<std::string, std::shared_ptr<ofFbo>> fontPreviews;
  std::shared_ptr<ofFbo> previewTempFbo;
  ofShader previewStrokeShader;
  ofTrueTypeFont previewFont;
  bool previewsInitialized = false;
  bool previewResourcesAllocated = false;
  static const int PreviewWidth = 250;
  static const int PreviewHeight = 250 * 0.5625;
  int currentPreviewWidth = 0;
  int currentPreviewHeight = 0;

  // Font Preview Helper Functions
  void initializePreviews();
  void allocatePreviewResources();
  void renderPreview(std::shared_ptr<ofFbo> targetFbo, const Font& fontToPreview);
  void updatePreviews();
  bool checkAndReallocateIfNeeded();
};

#endif /* TextEditorView_hpp */
