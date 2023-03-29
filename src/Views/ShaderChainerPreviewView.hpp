//
//  ShaderChainerPreviewView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/16/22.
//

#ifndef ShaderChainerPreviewView_hpp
#define ShaderChainerPreviewView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"
#include "VideoSourceService.hpp"

struct ShaderChainerPreviewView {
private:
  ofFbo previewFbo;
  int width;
  int height;
public:
  ShaderChainerPreviewView(std::shared_ptr<ShaderChainer> chainer) : chainer(chainer) {
    width = 1;
    height = 1;
  };
  std::shared_ptr<ShaderChainer> chainer;
  ImVec2 lastWindowSize;
  ImVec2 frameSize;
  void resize();
  void setup();
  void update();
  void draw();
};

#endif /* ShaderChainerPreviewView_hpp */
