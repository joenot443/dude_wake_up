//
//  ShaderBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "ShaderBrowserView.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"

TileBrowserView browserViewForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders)
{
  std::vector<std::shared_ptr<TileItem>> tileItems = {};
  for (auto shader : shaders)
  {
    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [shader]()
    {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("NewShader", &shader->type,
                                  sizeof(ShaderType));
        ImGui::Text("%s", shader->name.c_str());
        ImGui::EndDragDropSource();
      }
    };
    auto textureId = GetImTextureID(*shader->preview.get());
    auto tileItem = std::make_shared<TileItem>(shader->name, textureId, 0, dragCallback);

    tileItems.push_back(tileItem);
  }

  return TileBrowserView(tileItems);
}

void ShaderBrowserView::setup()
{
  auto basic = ShaderChainerService::getService()->availableBasicShaders;
  auto mix = ShaderChainerService::getService()->availableMixShaders;
  auto transform = ShaderChainerService::getService()->availableTransformShaders;
  auto filter = ShaderChainerService::getService()->availableFilterShaders;
  auto mask = ShaderChainerService::getService()->availableMaskShaders;

  basicTileBrowserView = browserViewForShaders(basic);
  mixTileBrowserView = browserViewForShaders(mix);
  transformTileBrowserView = browserViewForShaders(transform);
  filterTileBrowserView = browserViewForShaders(filter);
  maskTileBrowserView = browserViewForShaders(mask);
};

void ShaderBrowserView::draw()
{
  if (ImGui::BeginTabBar("VideoSourceBrowser", ImGuiTabBarFlags_None))
  {
    if (ImGui::BeginTabItem("Basic"))
    {
      basicTileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Filter"))
    {
      filterTileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Transform"))
    {
      transformTileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Mix"))
    {
      mixTileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Mask"))
    {
      maskTileBrowserView.draw();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
};

void ShaderBrowserView::update(){

};
