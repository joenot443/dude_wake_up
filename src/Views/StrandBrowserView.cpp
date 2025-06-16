//
//  StrandBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/23/24.
//

#include "StrandBrowserView.hpp"
#include "AvailableStrand.hpp"
#include "StrandService.hpp"

std::vector<std::shared_ptr<AvailableStrandTileItem>> StrandBrowserView::tileItemsFromAvailableStrands(std::vector<std::shared_ptr<AvailableStrand>> availableStrands)
{
  std::vector<std::shared_ptr<AvailableStrandTileItem>> items;
  int n = 0;
  for (auto const &availableStrand : availableStrands)
  {
    std::function <void(std::string)> dragCallback = [availableStrand](std::string tileId)
    {
      // Create a payload to carry the Strand
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("AvailableStrand", &availableStrand->id,
                                  sizeof(AvailableStrand));
        ImGui::PushID(tileId.c_str());
        ImGui::Text("%s", availableStrand->name.c_str());
        ImGui::PopID();
        ImGui::EndDragDropSource();
      }
    };
    ImTextureID textureId = (ImTextureID)(uint64_t) availableStrand->fbo->getTexture().texData.textureID;
    items.push_back(std::make_shared<AvailableStrandTileItem>(availableStrand, textureId, n, dragCallback));
    n++;
  }
  return items;
}

void StrandBrowserView::setup()
{
  // Get the available strands from the StrandService
  auto availableStrands = StrandService::getService()->availableStrands();

  // Create a vector of AvailableStrandTileItem objects from the available strands
  tileItems = tileItemsFromAvailableStrands(availableStrands);

  // Create a TileBrowserView with the tileItems
  tileBrowserView = TileBrowserView(tileItems);

  StrandService::getService()->subscribeToStrandsUpdated([this]()
  {
    tileItems = tileItemsFromAvailableStrands(StrandService::getService()->availableStrands());
    tileBrowserView = TileBrowserView(tileItems);
  });
}

void StrandBrowserView::update()
{
}

void StrandBrowserView::draw()
{
  // Begin a child window that fills the remaining space and allows vertical scrolling.
  // The 'true' argument adds a border around the child window.
  ImGui::BeginChild("##StrandBrowserScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

  // Draw the tile browser within this scrollable region.
  // Note: The layout (e.g., 2 columns, tile height) is typically handled
  // within the TileBrowserView::draw() method itself. You might need to
  // adjust TileBrowserView to achieve the desired 2x2 grid effect
  // where tiles are half the height of this scrolling region.
  tileBrowserView.draw();

  // End the child window.
  ImGui::EndChild();
}
