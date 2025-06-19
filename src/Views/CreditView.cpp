#include "CreditView.hpp"
#include "CommonViews.hpp"
#include "imgui.h"


void CreditView::draw(std::string popupId, std::shared_ptr<Credit> credit) {
  ImGui::SetNextWindowSize(ImVec2(400, 200));
  if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar)) {
    CommonViews::H3Title(credit->name);
    CommonViews::H3Title(credit->credit);
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
