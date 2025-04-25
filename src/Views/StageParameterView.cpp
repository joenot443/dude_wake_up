//
//  StageParameterView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#include "StageParameterView.hpp"
#include "CommonViews.hpp"
#include "OscillationService.hpp"
#include "FontService.hpp"

void StageParameterView::draw(std::shared_ptr<Parameter> param) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0, 5.0));
  switch (param->type) {
    case ParameterType_Color: {
      ImGui::BeginChild(param->paramId.c_str(), StageParameterViewSize, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
      CommonViews::ColorShaderStageParameter(param);
      ImGui::EndChild();
      break;
    }
    case ParameterType_Standard: {
      ImGui::BeginChild(param->paramId.c_str(), StageParameterViewSize, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
      CommonViews::ShaderStageParameter(param, OscillationService::getService()->oscillatorForParameter(param));
      ImGui::EndChild();
      break;
    }
    case ParameterType_Int: {
      ImGui::BeginChild(param->paramId.c_str(), StageParameterViewSize, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
      CommonViews::IntShaderStageParameter(param);
      ImGui::EndChild();
      break;
    }
      
    case ParameterType_Bool: {
      ImGui::BeginChild(param->paramId.c_str(), StageParameterViewSize, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
      CommonViews::H3Title(param->name);
      CommonViews::H4Title(param->ownerName);
      ImGui::PushFont(FontService::getService()->h3);
      bool old = param->boolValue;
      ImGui::Checkbox(idString(param->paramId).c_str(), &param->boolValue);
      bool ret = false;
      if (old != param->boolValue)
      {
        ret = true;
        param->setValue(static_cast<float>(param->boolValue));
      }
      ImGui::PopFont();
      ImGui::EndChild();
      break;
    }
    default:
      break;
  }
  ImGui::PopStyleVar();
}

