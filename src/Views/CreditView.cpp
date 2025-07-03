#include "CreditView.hpp"
#include "CommonViews.hpp"
#include "imgui.h"


void CreditView::draw(std::string popupId, std::shared_ptr<Credit> credit) {
  ImGui::SetNextWindowSize(ImVec2(400, 200));
  if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar)) {
    CommonViews::H3Title(credit->name);
    CommonViews::H3Title("Created by a community member named " + credit->credit);
    ImGui::SetNextItemWidth(380.0);
    ImGui::TextWrapped("If this is you and you'd like additional credit or if you'd prefer your work not be included at all, shoot me an email at joe@nottawa.app.");
    if (credit->description.length() > 0) {
      CommonViews::H4Title(credit->description);
    }
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2.0 - 10.0);
    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}
