//
//  OutputBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/18/22.
//

#include "OutputBrowserView.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourcePreviewView.hpp"
#include "CommonViews.hpp"

void OutputBrowserView::setup() {
  generatePreviewViews();
  ShaderChainerService::getService()->subscribeToShaderChainerUpdates([this](){
    generatePreviewViews();
  });
}

void OutputBrowserView::generatePreviewViews() {
  previewViews.clear();
 
 for (auto shaderChainer : ShaderChainerService::getService()->shaderChainers()) {
   VideoSourcePreviewView videoSourcePreviewView = VideoSourcePreviewView();
   videoSourcePreviewView.videoSource = shaderChainer;
   videoSourcePreviewView.setup();
   previewViews.push_back(videoSourcePreviewView);
 }
}

void OutputBrowserView::update() {
}

void OutputBrowserView::draw() {
  CommonViews::H3Title("Outputs");
  drawPreviewViews();
}

void OutputBrowserView::drawPreviewViews() {
 // Draw a VideoSourcePreviewView for every shaderChainer in the ShaderChainerService
  for (auto previewView : previewViews) {
    previewView.draw();
  }
}
