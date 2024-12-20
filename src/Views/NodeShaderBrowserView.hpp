#ifndef NodeShaderBrowserView_hpp
#define NodeShaderBrowserView_hpp

#include "ShaderType.hpp"
#include "PagedTileBrowserView.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

class NodeShaderBrowserView {
public:
  void setup();
  void draw();
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback);
  ImVec2 size = ImVec2(0.0, 0.0);
  float leftPadding = 0.0f;
  
private:
  void drawTabButtons();
  void drawSelectedBrowser();
  void applyButtonStyles(bool isSelected);
  void popButtonStyles(bool isSelected);
  
  std::unique_ptr<PagedTileBrowserView> basicTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> filterTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> glitchTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> transformTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> mixTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> maskTileBrowserView;
  
  int currentTab = 0;
  const char* tabLabels[6] = {"Basic", "Filter", "Glitch", "Size", "Mix", "Mask"};
};

#endif 
