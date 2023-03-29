//
//  ShaderChainerPreviewView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/16/22.
//

#include "ShaderChainerPreviewView.hpp"
#include "CommonViews.hpp"
#include "ShaderChainerService.hpp"

void ShaderChainerPreviewView::setup() {
  ofFbo::Settings fboSettings;
  fboSettings.width = width;
  fboSettings.height = height;
  fboSettings.internalformat = GL_RGB;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  previewFbo.allocate(fboSettings);
  previewFbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
}

void ShaderChainerPreviewView::update() {

  
  if (static_cast<int>(previewFbo.getHeight()) != height) {
    setup();
  }
}

void ShaderChainerPreviewView::draw() {
  if (chainer) {
    width = ImGui::GetWindowWidth() / 6. - 5;
    height = width / 1.3;
    CommonViews::ResolutionSelector(chainer);
    

    previewFbo.draw(ImGui::GetCursorPosX(), ImGui::GetCursorPosY(), width, height);
    
    previewFbo.begin();
    chainer->fbo.draw(0, 0, width, height);
    previewFbo.end();
    
    if (ImGui::Button(formatString("View##%s", chainer->sourceName.c_str()).c_str())) {
      VideoSourceService::getService()->addOutputWindowForChainer(chainer);
    }
    ImGui::SameLine();
    ImGui::Text(chainer->sourceName.c_str());
    
    CommonViews::Spacing(height / 2);

  }
}
