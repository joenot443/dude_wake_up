//
//  VideoSourcePreviewView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/16/22.
//

#ifndef VideoSourcePreviewView_hpp
#define VideoSourcePreviewView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"
#include "VideoSourceService.hpp"

struct VideoSourcePreviewView {
private:
  ofFbo previewFbo;
public:
  std::shared_ptr<VideoSource> videoSource;
  void setup();
  void update();
  void draw();
};

#endif /* VideoSourcePreviewView_hpp */
