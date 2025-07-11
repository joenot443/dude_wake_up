//
//  StrandBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/23/24.
//

#include "StrandBrowserView.hpp"
#include "AvailableStrand.hpp"
#include "StrandService.hpp"
#include "TileBrowserView.hpp"
#include <filesystem>
#include <cstring>

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

  // --------------------------------------------------------------------------
  // Setup context-menu actions
  std::vector<TileContextMenuItem> menuItems;

  // Delete action
  menuItems.emplace_back("Delete", [this](std::shared_ptr<TileItem> item) {
    auto strandItem = std::dynamic_pointer_cast<AvailableStrandTileItem>(item);
    if (!strandItem) return;
    try {
      // Remove file from disk if it exists
      if (std::filesystem::exists(strandItem->availableStrand->path)) {
        std::filesystem::remove(strandItem->availableStrand->path);
      }
    } catch (const std::exception &e) {
      log("Failed to delete Strand file: %s", e.what());
    }
    StrandService::getService()->removeStrand(strandItem->availableStrand->id);
  });

  // Rename action
  menuItems.emplace_back("Rename", [this](std::shared_ptr<TileItem> item) {
    auto strandItem = std::dynamic_pointer_cast<AvailableStrandTileItem>(item);
    if (!strandItem) return;
    strncpy(renameBuffer, strandItem->availableStrand->name.c_str(), sizeof(renameBuffer));
    renameBuffer[sizeof(renameBuffer) - 1] = '\0';
    renameStrandId = strandItem->availableStrand->id;
    showRenamePopup = true;
  });

  tileBrowserView.setContextMenuItems(menuItems);

  StrandService::getService()->subscribeToStrandsUpdated([this]()
  {
    tileItems = tileItemsFromAvailableStrands(StrandService::getService()->availableStrands());
    tileBrowserView.setTileItems(std::vector<std::shared_ptr<TileItem>>(tileItems.begin(), tileItems.end()));
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

  // --------------------------------------------------------------------------
  // Show helper text when there are no strands -----------------------------------------------------
  if (tileItems.empty())
  {
    ImGui::Dummy(ImVec2(1.0f, 5.0f));
    ImGui::Dummy(ImVec2(5.0f, 1.0f));
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
    ImGui::TextWrapped("No strands found. Save your creations by right clicking a node and selecting Save Strand.");
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(1.0f, 5.0f));
  }
  else
  {
    tileBrowserView.draw();
  }

  ImGui::EndChild();

  // --------------------------------------------------------------------------
  // Rename popup
  if (showRenamePopup)
  {
    ImGui::OpenPopup("Rename Strand");
    showRenamePopup = false;
  }

  if (ImGui::BeginPopupModal("Rename Strand", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::InputText("New name", renameBuffer, IM_ARRAYSIZE(renameBuffer));

    if (ImGui::Button("OK", ImVec2(120, 0)))
    {
      std::string newName(renameBuffer);
      if (!newName.empty())
      {
        StrandService::getService()->renameStrand(renameStrandId, newName);
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}
