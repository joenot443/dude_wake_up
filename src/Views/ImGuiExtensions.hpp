//
//  ImGuiExtensions.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/27/23.
//

#ifndef ImGuiExtensions_hpp
#define ImGuiExtensions_hpp

#include <stdio.h>
#include <imgui.h>

struct ImGuiExtensions {
  static bool Slider2DScalar(char const* pLabel, ImGuiDataType data_type, void* p_valueX, void* p_valueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY, float const fScale);
  
  static bool Slider2DFloat(char const* pLabel, float* pValueX, float* pValueY, float minX, float maxX, float minY, float maxY, float const fScale);
  
  static bool RangeSelect2D(char const* pLabel, float* pCurMinX, float* pCurMinY, float* pCurMaxX, float* pCurMaxY, float const fBoundMinX, float const fBoundMinY, float const fBoundMaxX, float const fBoundMaxY, float const fScale /*= 1.0f*/);
};

namespace ImGui
{
bool RangeSliderFloat(const char* label, float* v1, float* v2, float v_min, float v_max, const char* display_format, float power);

void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f));

void EndGroupPanel();
};


#endif /* ImGuiExtensions_hpp */
