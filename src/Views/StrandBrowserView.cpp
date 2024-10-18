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
    std::function <void()> dragCallback = [availableStrand]()
    {
      // Create a payload to carry the Strand
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("AvailableStrand", &availableStrand->id,
                                  sizeof(AvailableStrand));
        ImGui::Text("%s", availableStrand->name.c_str());
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
  tileBrowserView.draw();
}
