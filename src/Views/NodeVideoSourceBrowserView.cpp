#include "NodeVideoSourceBrowserView.hpp"
#include "VideoSourceService.hpp"
#include "LibraryService.hpp"
#include "Colors.hpp"

void NodeVideoSourceBrowserView::setup() {
  generatedTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  webcamTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  libraryTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  
  // Set sizes and padding
  for (auto* view : {&generatedTileBrowserView, &webcamTileBrowserView, &libraryTileBrowserView}) {
    (*view)->setWidth(size.x);
    (*view)->leftPadding = leftPadding;
  }
  
  VideoSourceService::getService()->subscribeToAvailableVideoSourceUpdates(
    [this](){ refreshSources(); });
  ofAddListener(LibraryService::getService()->thumbnailNotification, this, &NodeVideoSourceBrowserView::refreshSources);
  
  refreshSources();
}

void NodeVideoSourceBrowserView::applyButtonStyles(bool isSelected) {
  if (isSelected) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::ButtonSelected.Value);
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // transparent
  }
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::ButtonSelectedHovered.Value);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::ButtonSelected.Value);
  ImGui::PushStyleColor(ImGuiCol_Border, Colors::NodeBorderColor.Value);
}

void NodeVideoSourceBrowserView::popButtonStyles(bool isSelected) {
  if (isSelected) {
    ImGui::PopStyleVar(2);
  }
  ImGui::PopStyleColor(4);
}

void NodeVideoSourceBrowserView::drawTabButtons() {
  ImGui::Dummy(ImVec2(leftPadding, 0.0));
  ImGui::SameLine();
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
  
  float buttonWidth = (size.x - leftPadding) / 3.0f;
  for (int i = 0; i < 3; i++) {
    if (i > 0) ImGui::SameLine();
    
    bool isSelected = (currentTab == i);
    applyButtonStyles(isSelected);
    
    if (ImGui::Button(tabLabels[i], ImVec2(buttonWidth, 0))) {
      currentTab = i;
    }
    
    popButtonStyles(isSelected);
  }
  
  ImGui::PopStyleVar(); // Pop ItemSpacing
  ImGui::Dummy(ImVec2(0.0, 10.0));
}

void NodeVideoSourceBrowserView::drawSelectedBrowser() {
  switch (currentTab) {
    case 0: generatedTileBrowserView->draw(); break;
    case 1: webcamTileBrowserView->draw(); break;
    case 2: libraryTileBrowserView->draw(); break;
  }
}

void NodeVideoSourceBrowserView::draw() {
  drawTabButtons();
  ImGui::Spacing();
  drawSelectedBrowser();
  ImGui::NewLine();
}

void NodeVideoSourceBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  generatedTileBrowserView->setCallback(callback);
  webcamTileBrowserView->setCallback(callback);
  libraryTileBrowserView->setCallback(callback);
}

void NodeVideoSourceBrowserView::refreshSources()
{
  shaderItems.clear();
  libraryItems.clear();
  webcamItems.clear();
  auto sources = VideoSourceService::getService()->availableVideoSources();
  for (auto source : sources)
  {
    if (!source->hasPreview)
    {
      source->generatePreview();
    }

    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [source]()
    {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("VideoSource", &source.get()->availableVideoSourceId,
                                  sizeof(std::string));
        ImGui::Text("%s", source->sourceName.c_str());
        ImGui::EndDragDropSource();
      }
    };
    ImTextureID textureId = (ImTextureID)(uint64_t) source->preview->texData.textureID;
    ShaderType type = ShaderTypeNone;
    if (source->type == VideoSource_shader) {
      type = shaderTypeForShaderSourceType(std::dynamic_pointer_cast<AvailableVideoSourceShader>(source)->shaderType);
    }
    auto tileItem = std::make_shared<TileItem>(source->sourceName, textureId, 0, dragCallback, source->category, TileType_Source, type);

    if (source->type == VideoSource_shader || 
        source->type == VideoSource_text ||
        source->type == VideoSource_multi ||
        source->type == VideoSource_icon)
    {
      shaderItems.push_back(tileItem);
    }
    else if (source->type == VideoSource_library)
    {
      if (std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(source) != nullptr)
      {
        auto librarySource = std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(source);
        auto libraryTileItem = std::make_shared<LibraryTileItem>(source->sourceName, textureId, 0, librarySource->libraryFile, dragCallback, librarySource->libraryFile->category);
        libraryItems.push_back(libraryTileItem);
      }
    }
    else if (source->type == VideoSource_webcam)
    {
      webcamItems.push_back(tileItem);
    }
  }
  // Sort the shader items by category count and then by name
  
  // Count the number of times each category appears in the tileItems
  std::map<std::string, int> categoryCounts;
  for (const auto &tileItem : shaderItems) {
    categoryCounts[tileItem->category]++;
  }
  
  // Sort the tileItems based on the number of times each category appears
  std::sort(shaderItems.begin(), shaderItems.end(),
            [&](const auto &a, const auto &b) {
    // Compare based on category counts
    if (categoryCounts[a->category] != categoryCounts[b->category]) {
      return categoryCounts[a->category] > categoryCounts[b->category];
    }
    if (a->category == b->category) {
      return a->name < b->name;
    }
    // If counts are equal, compare categories alphabetically
    return a->category < b->category;
  });
  
  
  std::sort(libraryItems.begin(), libraryItems.end(),
            [&](const auto &a, const auto &b) {
    // Compare based on category counts
    if (categoryCounts[a->category] != categoryCounts[b->category]) {
      return categoryCounts[a->category] > categoryCounts[b->category];
    }
    if (a->category == b->category) {
      return a->name < b->name;
    }
    // If counts are equal, compare categories alphabetically
    return a->category < b->category;
  });

  // Sort the webcam items by name
  std::sort(webcamItems.begin(), webcamItems.end(),
            [](std::shared_ptr<TileItem> a, std::shared_ptr<TileItem> b)
            { return a->name < b->name; });

  generatedTileBrowserView->setTileItems(shaderItems);
  webcamTileBrowserView->setTileItems(webcamItems);
  auto libraryItems = std::vector<std::shared_ptr<LibraryTileItem>>();
  for (auto item : libraryItems) {
    libraryItems.push_back(item);
  }
  // Cast to vector of shared_ptr<TileItem>
  std::vector<std::shared_ptr<TileItem>> libraryItems2;
  for (auto item : libraryItems) {
    libraryItems2.push_back(item);
  }
  libraryTileBrowserView->setTileItems(libraryItems2);
}

void NodeVideoSourceBrowserView::update() {}
