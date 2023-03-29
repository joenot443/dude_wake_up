//
//  OutputBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/18/22.
//

#include "OutputBrowserView.hpp"
#include "ShaderChainerService.hpp"
#include "ShaderChainerPreviewView.hpp"
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
   auto shaderChainerPreviewView = std::make_shared<ShaderChainerPreviewView>(shaderChainer);
   shaderChainerPreviewView->setup();
   previewViews.push_back(shaderChainerPreviewView);
 }
}

void OutputBrowserView::update() {
  for (auto view : previewViews) {
    view->update();
  }
}

void OutputBrowserView::draw() {
  CommonViews::H3Title("Outputs");
  drawPreviewViews();
}

void OutputBrowserView::drawPreviewViews() {
 // Draw a ShaderChainerPreviewView for every shaderChainer in the ShaderChainerService
  for (auto previewView : previewViews) {
    previewView->draw();
  }
}
