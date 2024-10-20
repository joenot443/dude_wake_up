//
//  ShaderBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "ShaderBrowserView.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"
#include "CommonViews.hpp"

std::shared_ptr<TileItem> tileItemForShader(std::shared_ptr<AvailableShader> shader) {
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
  ImTextureID textureId = (ImTextureID)(uint64_t) shader->preview->texData.textureID;
  return std::make_shared<TileItem>(shader->name, textureId, 0, dragCallback, "", shader->type);
}

std::vector<std::shared_ptr<TileItem>> tileItemsForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders) {
  std::vector<std::shared_ptr<TileItem>> tileItems = {};
  for (auto shader : shaders)
  {
    tileItems.push_back(tileItemForShader(shader));
  }
  return tileItems;
}

TileBrowserView browserViewForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders)
{
  return TileBrowserView(tileItemsForShaders(shaders));
}

void ShaderBrowserView::setup()
{
  auto basic = ShaderChainerService::getService()->availableBasicShaders;
  auto mix = ShaderChainerService::getService()->availableMixShaders;
  auto transform = ShaderChainerService::getService()->availableTransformShaders;
  auto filter = ShaderChainerService::getService()->availableFilterShaders;
  auto mask = ShaderChainerService::getService()->availableMaskShaders;
  auto favorites = ShaderChainerService::getService()->availableFavoriteShaders();
  auto defaultFavorites = ShaderChainerService::getService()->availableDefaultFavoriteShaders;
  auto glitch = ShaderChainerService::getService()->availableGlitchShaders;
  
  searchResultsTileBrowserView = TileBrowserView(searchTileItems);
  basicTileBrowserView = browserViewForShaders(basic);
  mixTileBrowserView = browserViewForShaders(mix);
  transformTileBrowserView = browserViewForShaders(transform);
  filterTileBrowserView = browserViewForShaders(filter);
  maskTileBrowserView = browserViewForShaders(mask);
  favoritesTileBrowserView = browserViewForShaders(favorites);
  defaultFavoritesTileBrowserView = browserViewForShaders(defaultFavorites);
  glitchTileBrowserView = browserViewForShaders(glitch);
};

void ShaderBrowserView::draw()
{
  char buffer[256];
  strncpy(buffer, searchQuery.c_str(), sizeof(buffer));
  if (ImGui::InputText("Search", buffer, sizeof(buffer))) {
    searchQuery = std::string(buffer);
    searchDirty = true;
  }
  
  if (searchQuery.length() != 0) {
    searchResultsTileBrowserView.draw();
    return;
  }
  
  if (ImGui::BeginTabBar("VideoSourceBrowser", ImGuiTabBarFlags_FittingPolicyScroll))
  {
    if (ImGui::BeginTabItem("Favorites"))
    {
      CommonViews::H4Title("Default Favorites");
      defaultFavoritesTileBrowserView.draw();
      CommonViews::H4Title("Your Favorites");
      favoritesTileBrowserView.draw();
      ImGui::EndTabItem();
    }
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
    if (ImGui::BeginTabItem("Glitch"))
    {
      glitchTileBrowserView.draw();
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
  if (searchDirty) {
    // Filter tileItems based on searchQuery
    auto& allShaders = ShaderChainerService::getService()->allAvailableShaders;
    std::vector<std::shared_ptr<TileItem>> filteredItems;

    // Convert the searchQuery to lowercase for case-insensitive comparison
    std::string searchQueryLower = searchQuery;
    std::transform(searchQueryLower.begin(), searchQueryLower.end(), searchQueryLower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    for (auto& shader : allShaders) {
        // Convert shader name to lowercase for case-insensitive comparison
        std::string shaderNameLower = shader->name;
        std::transform(shaderNameLower.begin(), shaderNameLower.end(), shaderNameLower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (shaderNameLower.find(searchQueryLower) != std::string::npos) {
            filteredItems.push_back(tileItemForShader(shader));
        }
    }
    searchResultsTileBrowserView.setTileItems(filteredItems);
    searchDirty = false;
  }
};
