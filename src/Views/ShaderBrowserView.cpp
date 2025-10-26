//
//  ShaderBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "ShaderBrowserView.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"
#include "PagedTileBrowserView.hpp"
#include "CommonViews.hpp"

TileBrowserView browserViewForShaders(std::vector<std::shared_ptr<AvailableShader>> shaders)
{
  return TileBrowserView(tileItemsForShaders(shaders));
}

void ShaderBrowserView::setup()
{
  auto service = ShaderChainerService::getService();
  auto basic = service->availableBasicShaders;
  auto mix = service->availableMixShaders;
  auto transform = service->availableTransformShaders;
  auto filter = service->availableFilterShaders;
  auto mask = service->availableMaskShaders;
  auto favorites = service->availableFavoriteShaders();
  auto defaultFavorites = service->availableDefaultFavoriteShaders;
  auto glitch = service->availableGlitchShaders;
  
  searchResultsTileBrowserView = std::make_unique<TileBrowserView>(searchTileItems);
  basicTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(basic));
  mixTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(mix));
  transformTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(transform));
  filterTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(filter));
  maskTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(mask));
  std::vector<std::shared_ptr<TileItem>> favoriteTileItems = tileItemsForShaders(favorites, "My Favorites");
  std::vector<std::shared_ptr<TileItem>> defaultFavoriteTileItems = tileItemsForShaders(defaultFavorites, "Default Favorites");

  favoriteTileItems.insert(favoriteTileItems.end(), defaultFavoriteTileItems.begin(), defaultFavoriteTileItems.end());
  favoritesTileBrowserView = std::make_unique<TileBrowserView>(favoriteTileItems);
  glitchTileBrowserView = std::make_unique<TileBrowserView>(tileItemsForShaders(glitch));

  // Set sizes
  for (auto* view : {&searchResultsTileBrowserView, &basicTileBrowserView, &mixTileBrowserView,
                     &transformTileBrowserView, &filterTileBrowserView, &maskTileBrowserView,
                     &favoritesTileBrowserView, &glitchTileBrowserView}) {
    (*view)->size = size;
  }
  
  // Set tile items
  searchResultsTileBrowserView->setTileItems(searchTileItems);
  basicTileBrowserView->setTileItems(tileItemsForShaders(basic));
  mixTileBrowserView->setTileItems(tileItemsForShaders(mix));
  transformTileBrowserView->setTileItems(tileItemsForShaders(transform));
  filterTileBrowserView->setTileItems(tileItemsForShaders(filter));
  maskTileBrowserView->setTileItems(tileItemsForShaders(mask));
  favoritesTileBrowserView->setTileItems(favoriteTileItems);
//  defaultFavoritesTileBrowserView->setTileItems(tileItemsForShaders(defaultFavorites));
  glitchTileBrowserView->setTileItems(tileItemsForShaders(glitch));
  
  currentTab = 0;
}

void ShaderBrowserView::drawSearchView() {
  CommonViews::CollapsibleSearchHeader(
    "Effects",
    "Search Effects",
    collapsed,
    searchQuery,
    searchDirty,
    "ShaderSearchClear"
  );
  
  if (searchQuery.length() != 0) {
    searchResultsTileBrowserView->draw();
  }
  
  if (searchQuery.length() != 0 && searchTileItems.size() > 0) {
    searchResultsTileBrowserView->draw();
  } else if (searchQuery.length() > 0) {
    ImGui::Dummy(ImVec2(1.0, 5.0));
    ImGui::Dummy(ImVec2(5.0, 1.0));
    ImGui::SameLine();
    ImGui::Text("No results.");
    ImGui::Dummy(ImVec2(1.0, 5.0));
  }
}

void ShaderBrowserView::setCurrentTab(int tabIndex) {
  currentTab = tabIndex;
}

void ShaderBrowserView::drawSelectedBrowser() {
  ImGui::BeginChild("##selectedBrowser", ImVec2(ImGui::GetWindowWidth() - 20.0, ImGui::GetWindowHeight() - 80.0), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);
  switch (currentTab) {
    case 0: // Favorites
      favoritesTileBrowserView->draw();
      break;
    case 1: // Basic
      basicTileBrowserView->draw();
      break;
    case 2: // Filter
      filterTileBrowserView->draw();
      break;
    case 3: // Glitch
      glitchTileBrowserView->draw();
      break;
    case 4: // Transform
      transformTileBrowserView->draw();
      break;
    case 5: // Mix
      mixTileBrowserView->draw();
      break;
    case 6: // Mask
      maskTileBrowserView->draw();
      break;
  }
  ImGui::EndChild();
}

void ShaderBrowserView::draw()
{
  drawSearchView();

  // Only draw tabs and content if not collapsed
  if (collapsed == nullptr || !*collapsed) {
    if (ImGui::BeginTabBar(idAppendedToString("VideoSourceBrowser", browserId).c_str(), ImGuiTabBarFlags_FittingPolicyScroll)) {
      if (ImGui::BeginTabItem(idAppendedToString("Favorites", browserId).c_str(), nullptr, currentTab == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 0;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Basic", browserId).c_str(), nullptr, currentTab == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 1;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Filter", browserId).c_str(), nullptr, currentTab == 2 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 2;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Glitch", browserId).c_str(), nullptr, currentTab == 3 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 3;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Transform", browserId).c_str(), nullptr, currentTab == 4 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 4;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Mix", browserId).c_str(), nullptr, currentTab == 5 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 5;
      }

      if (ImGui::BeginTabItem(idAppendedToString("Mask", browserId).c_str(), nullptr, currentTab == 6 ? ImGuiTabItemFlags_SetSelected : 0)) {
        drawSelectedBrowser();
        ImGui::EndTabItem();
      }
      if (ImGui::IsItemClicked()) {
        currentTab = 6;
      }

      ImGui::EndTabBar();
    }
  }
}

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
    searchResultsTileBrowserView->setTileItems(filteredItems);
    searchDirty = false;
  }
};

void ShaderBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  searchResultsTileBrowserView->setCallback(callback);
  basicTileBrowserView->setCallback(callback);
  mixTileBrowserView->setCallback(callback);
  transformTileBrowserView->setCallback(callback);
  filterTileBrowserView->setCallback(callback);
  maskTileBrowserView->setCallback(callback);
  favoritesTileBrowserView->setCallback(callback);
  defaultFavoritesTileBrowserView->setCallback(callback);
  glitchTileBrowserView->setCallback(callback);
}
