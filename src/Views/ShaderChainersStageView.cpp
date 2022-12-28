//
//  ShaderChainersStageView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/22/22.
//

#include "ShaderChainersStageView.hpp"
#include "ShaderChainerService.hpp"

void ShaderChainersStageView::setup() {
  generateChainerViews();
  ShaderChainerService::getService()->subscribeToShaderChainerUpdates([this](){
    generateChainerViews();
  });
}

void ShaderChainersStageView::update() {
  
}

void ShaderChainersStageView::draw() {
  // Iterate through the shader chainers
  for (auto shaderChainerView : chainerViews) {
    shaderChainerView.draw();
  }
}

void ShaderChainersStageView::generateChainerViews() {
  chainerViews.clear();

  auto shaderChainers = ShaderChainerService::getService()->shaderChainers();

  for (auto c : shaderChainers) {
    auto shaderChainerView = ShaderChainerView(c);
    chainerViews.push_back(shaderChainerView);
  }

  std::sort(chainerViews.begin(), chainerViews.end(),
            [](const ShaderChainerView &a,
               const ShaderChainerView &b) {
              return a.shaderChainer->name < b.shaderChainer->name;
            });
}
