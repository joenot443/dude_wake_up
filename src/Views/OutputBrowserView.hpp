//
//  OutputBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/18/22.
//

#ifndef OutputBrowserView_hpp
#define OutputBrowserView_hpp

#include <stdio.h>
#include "ShaderChainerPreviewView.hpp"

struct OutputBrowserView {
public:
  void setup();
  void update();
  void draw();
  
private:
  void drawPreviewViews();
  void generatePreviewViews();
  
  std::vector<std::shared_ptr<ShaderChainerPreviewView>> previewViews;
};

#endif /* OutputBrowserView_hpp */
