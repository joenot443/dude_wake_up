//
//  ShaderBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "ShaderBrowserView.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"

void ShaderBrowserView::setup(){
  auto shaders = ShaderChainerService::getService()->availableShaders;

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

  tileBrowserView = TileBrowserView(tileItems);
};

void ShaderBrowserView::draw() {
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Effects");
  ImGui::PopFont();

  tileBrowserView.draw();
};

void ShaderBrowserView::update(){

};
