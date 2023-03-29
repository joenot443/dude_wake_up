//
//  ShaderBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "ShaderBrowserView.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"

TileBrowserView browserViewForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders) {
  std::vector<TileItem> tileItems = {};
  for (auto shader : shaders) {
    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [shader]() {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("NewShader", &shader->type,
                                  sizeof(ShaderType));
        ImGui::Text("%s", shader->name.c_str());
        ImGui::EndDragDropSource();
      }
    };
    auto textureId = GetImTextureID(*shader->preview.get());
    TileItem tileItem = TileItem(shader->name, textureId, 0, dragCallback);

    tileItems.push_back(tileItem);
  }

  return TileBrowserView(tileItems);
}

void ShaderBrowserView::setup(){
  auto basic = ShaderChainerService::getService()->availableBasicShaders;
  auto mix = ShaderChainerService::getService()->availableMixShaders;
  auto transform = ShaderChainerService::getService()->availableTransformShaders;
  auto filter = ShaderChainerService::getService()->availableFilterShaders;

  basicTileBrowserView = browserViewForShaders(basic);
  mixTileBrowserView = browserViewForShaders(mix);
  transformTileBrowserView = browserViewForShaders(transform);
  filterTileBrowserView = browserViewForShaders(filter);
};


void ShaderBrowserView::draw() {
  CommonViews::H3Title("Effects");
  CommonViews::H4Title("Basic");
  basicTileBrowserView.draw();
  
  CommonViews::H4Title("Mix");
  mixTileBrowserView.draw();
  
  CommonViews::H4Title("Transform");
  transformTileBrowserView.draw();
  
  CommonViews::H4Title("Filter");
  filterTileBrowserView.draw();
};

void ShaderBrowserView::update(){

};
