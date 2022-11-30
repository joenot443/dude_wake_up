//
//  OutputBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/18/22.
//

#include "OutputBrowserView.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourcePreviewView.hpp"

void OutputBrowserView::setup() {
}

void OutputBrowserView::update() {
  if (previewViews.size() != ShaderChainerService::getService()->count()) {
     previewViews.clear();
    
    for (auto shaderChainer : ShaderChainerService::getService()->shaderChainers()) {
      VideoSourcePreviewView videoSourcePreviewView = VideoSourcePreviewView();
      videoSourcePreviewView.videoSource = shaderChainer;
      videoSourcePreviewView.setup();
      previewViews.push_back(videoSourcePreviewView);
    }
  }
}

void OutputBrowserView::draw() {
  drawOutputs();
}

void OutputBrowserView::drawOutputs() {
 // Draw a VideoSourcePreviewView for every shaderChainer in the ShaderChainerService
  for (auto previewView : previewViews) {
    previewView.draw();
  }
}
