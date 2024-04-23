//
//  StageParameterView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#include "StageParameterView.hpp"
#include "CommonViews.hpp"
#include "OscillationService.hpp"

void StageParameterView::draw(std::shared_ptr<FavoriteParameter> favoriteParameter) {
  ImGui::BeginChild(favoriteParameter->parameter->paramId.c_str(), StageParameterViewSize);
  CommonViews::ShaderStageParameter(favoriteParameter, OscillationService::getService()->oscillatorForParameter(favoriteParameter->parameter));
  ImGui::EndChild();
}

