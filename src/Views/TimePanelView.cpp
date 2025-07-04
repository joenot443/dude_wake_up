#include "TimePanelView.hpp"
#include "CommonViews.hpp"
#include "imgui.h"
#include "TimeService.hpp"

void TimePanelView::setup() {
  // Setup code if needed
}

void TimePanelView::update() {
  // Update code if needed
}

void TimePanelView::draw() {
  ImGui::BeginChild("##timePanel", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
  CommonViews::H3Title("Time");
  CommonViews::ShaderParameter(TimeService::getService()->timeSpeedParam, TimeService::getService()->timeSpeedOscillator);
  CommonViews::ShaderParameter(TimeService::getService()->timeOffsetParam, TimeService::getService()->timeOffsetOscillator);
  ImGui::EndChild();
}
