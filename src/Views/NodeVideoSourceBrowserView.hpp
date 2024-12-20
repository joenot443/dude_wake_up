#ifndef NodeVideoSourceBrowserView_hpp
#define NodeVideoSourceBrowserView_hpp

#include "PagedTileBrowserView.hpp"
#include "FileBrowserView.hpp"
#include "VideoSource.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

class NodeVideoSourceBrowserView {
public:
  void setup();
  void draw();
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback);
  void refreshSources();
  
  ImVec2 size = ImVec2(0.0, 0.0);
  float leftPadding = 0.0f;
  
private:
  void drawTabButtons();
  void drawSelectedBrowser();
  void applyButtonStyles(bool isSelected);
  void popButtonStyles(bool isSelected);
  
  std::unique_ptr<PagedTileBrowserView> generatedTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> webcamTileBrowserView;
  std::unique_ptr<PagedTileBrowserView> libraryTileBrowserView;
  std::unique_ptr<FileBrowserView> fileBrowserView;
  
  int currentTab = 0;
  const char* tabLabels[4] = {"Generated", "Webcam", "Library", "Files"};
  
  std::vector<std::shared_ptr<TileItem>> webcamItems;
  std::vector<std::shared_ptr<TileItem>> shaderItems;
  std::vector<std::shared_ptr<LibraryTileItem>> libraryItems;
};

#endif
