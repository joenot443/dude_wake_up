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
};

#endif /* ImGuiExtensions_hpp */
