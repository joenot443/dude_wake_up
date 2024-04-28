//
//  ImGuiExtensions.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/27/23.
//

#include "CommonViews.hpp"
#include "ImGuiExtensions.hpp"
#include <imgui.h>
#include <imgui_internal.h>

float Rescale01(float const x, float const min, float const max)
{
  return (x - min) / (max - min);
}
ImVec2 Rescale01v(ImVec2 const v, ImVec2 const min, ImVec2 const max)
{
  return ImVec2(Rescale01(v.x, min.x, max.x), Rescale01(v.y, min.y, max.y));
}
float Rescale(float const x, float const min, float const max, float const newMin, float const newMax)
{
  return Rescale01(x, min, max) * (newMax - newMin) + newMin;
}
ImVec2 Rescalev(ImVec2 const x, ImVec2 const min, ImVec2 const max, ImVec2 const newMin, ImVec2 const newMax)
{
  ImVec2 const vNorm = Rescale01v(x, min, max);
  return ImVec2(vNorm.x * (newMax.x - newMin.x) + newMin.x, vNorm.y * (newMax.y - newMin.y) + newMin.y);
}
float Sign(float x)
{
  return x < 0.0f ? -1.0f : 1.0f;
}

bool  IsNegativeScalar(ImGuiDataType data_type, ImU64* src)
{
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = *reinterpret_cast<ImS8*>(src);
      return value < 0;
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(src);
      return value < 0;
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(src);
      return value < 0l;
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(src);
      return value < 0ll;
    }
      break;
    case ImGuiDataType_U8:
    case ImGuiDataType_U16:
    case ImGuiDataType_U32:
    case ImGuiDataType_U64:
    {
      return false;
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(src);
      return value < 0.0f;
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(src);
      return value < 0.0f;
    }
      break;
  }
  
  return false;
}

bool  IsPositiveScalar(ImGuiDataType data_type, ImU64* src)
{
  return !IsNegativeScalar(data_type, src);
}

void  EqualScalar(ImGuiDataType data_type, ImU64* p_target, ImU64* p_source)
{
  switch (data_type)
  {
    case ImGuiDataType_S8:
      *reinterpret_cast<ImS8*>(p_target) = *reinterpret_cast<ImS8*>(p_source);
      break;
    case ImGuiDataType_U8:
      *reinterpret_cast<ImU8*>(p_target) = *reinterpret_cast<ImU8*>(p_source);
      break;
    case ImGuiDataType_S16:
      *reinterpret_cast<ImS16*>(p_target) = *reinterpret_cast<ImS16*>(p_source);
      break;
    case ImGuiDataType_U16:
      *reinterpret_cast<ImU16*>(p_target) = *reinterpret_cast<ImU16*>(p_source);
      break;
    case ImGuiDataType_S32:
      *reinterpret_cast<ImS32*>(p_target) = *reinterpret_cast<ImS32*>(p_source);
      break;
    case ImGuiDataType_U32:
      *reinterpret_cast<ImU64*>(p_target) = *reinterpret_cast<ImU32*>(p_source);
      break;
    case ImGuiDataType_S64:
      *reinterpret_cast<ImS64*>(p_target) = *reinterpret_cast<ImS64*>(p_source);
      break;
    case ImGuiDataType_U64:
      *reinterpret_cast<ImU64*>(p_target) = *reinterpret_cast<ImU64*>(p_source);
      break;
    case ImGuiDataType_Float:
      *reinterpret_cast<float*>(p_target) = *reinterpret_cast<float*>(p_source);
      break;
    case ImGuiDataType_Double:
      *reinterpret_cast<double*>(p_target) = *reinterpret_cast<double*>(p_source);
      break;
  }
}


ImU64  MulScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = *reinterpret_cast<ImS8*>(p_a) * *static_cast<ImS8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 value = *reinterpret_cast<ImU8*>(p_a) * *static_cast<ImU8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(p_a) * *static_cast<ImS16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 value = *reinterpret_cast<ImU16*>(p_a) * *static_cast<ImU16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(p_a) * *static_cast<ImS32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 value = *reinterpret_cast<ImU32*>(p_a) * *static_cast<ImU32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(p_a) * *static_cast<ImS64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 value = *reinterpret_cast<ImU64*>(p_a) * *static_cast<ImU64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(p_a) * *static_cast<float*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) * *static_cast<double*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
  }
  
  return result;
}

ImU64  DivScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = *reinterpret_cast<ImS8*>(p_a) / *static_cast<ImS8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 value = *reinterpret_cast<ImU8*>(p_a) / *static_cast<ImU8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(p_a) / *static_cast<ImS16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 value = *reinterpret_cast<ImU16*>(p_a) / *static_cast<ImU16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(p_a) / *static_cast<ImS32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 value = *reinterpret_cast<ImU32*>(p_a) / *static_cast<ImU32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(p_a) / *static_cast<ImS64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 value = *reinterpret_cast<ImU64*>(p_a) / *static_cast<ImU64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(p_a) / *static_cast<float*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) / *static_cast<double*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
  }
  
  return result;
}

ImU64  ClampScalar(ImGuiDataType data_type, void* p_value, void* p_min, void* p_max)
{
  ImU64 result = 0;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = ImClamp(*reinterpret_cast<ImS8*>(p_value), *static_cast<ImS8*>(p_min), *static_cast<ImS8*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 value = ImClamp(*reinterpret_cast<ImU8*>(p_value), *static_cast<ImU8*>(p_min), *static_cast<ImU8*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = ImClamp(*reinterpret_cast<ImS16*>(p_value), *static_cast<ImS16*>(p_min), *static_cast<ImS16*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 value = ImClamp(*reinterpret_cast<ImU16*>(p_value), *static_cast<ImU16*>(p_min), *static_cast<ImU16*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = ImClamp(*reinterpret_cast<ImS32*>(p_value), *static_cast<ImS32*>(p_min), *static_cast<ImS32*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 value = ImClamp(*reinterpret_cast<ImU32*>(p_value), *static_cast<ImU32*>(p_min), *static_cast<ImU32*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = ImClamp(*reinterpret_cast<ImS64*>(p_value), *static_cast<ImS64*>(p_min), *static_cast<ImS64*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 value = ImClamp(*reinterpret_cast<ImU64*>(p_value), *static_cast<ImU64*>(p_min), *static_cast<ImU64*>(p_max));
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = ImClamp(*reinterpret_cast<float*>(p_value), *static_cast<float*>(p_min), *static_cast<float*>(p_max));
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = ImClamp(*reinterpret_cast<double*>(p_value), *static_cast<double*>(p_min), *static_cast<double*>(p_max));
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
  }
  
  return result;
}


ImU64  AddScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = *reinterpret_cast<ImS8*>(p_a) + *static_cast<ImS8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 value = *reinterpret_cast<ImU8*>(p_a) + *static_cast<ImU8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(p_a) + *static_cast<ImS16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 value = *reinterpret_cast<ImU16*>(p_a) + *static_cast<ImU16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(p_a) + *static_cast<ImS32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 value = *reinterpret_cast<ImU32*>(p_a) + *static_cast<ImU32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(p_a) + *static_cast<ImS64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 value = *reinterpret_cast<ImU64*>(p_a) + *static_cast<ImU64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(p_a) + *static_cast<float*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) + *static_cast<double*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
  }
  
  return result;
}



float  ScalarToFloat(ImGuiDataType data_type, ImU64* p_source)
{
  switch (data_type)
  {
    case ImGuiDataType_S8:
      return static_cast<float>(*reinterpret_cast<ImS8*>(p_source));
    case ImGuiDataType_U8:
      return static_cast<float>(*reinterpret_cast<ImU8*>(p_source));
    case ImGuiDataType_S16:
      return static_cast<float>(*reinterpret_cast<ImS16*>(p_source));
    case ImGuiDataType_U16:
      return static_cast<float>(*reinterpret_cast<ImU16*>(p_source));
    case ImGuiDataType_S32:
      return static_cast<float>(*reinterpret_cast<ImS32*>(p_source));
    case ImGuiDataType_U32:
      return static_cast<float>(*reinterpret_cast<ImU32*>(p_source));
    case ImGuiDataType_S64:
      return static_cast<float>(*reinterpret_cast<ImS64*>(p_source));
    case ImGuiDataType_U64:
      return static_cast<float>(*reinterpret_cast<ImU64*>(p_source));
    case ImGuiDataType_Float:
      return static_cast<float>(*reinterpret_cast<float*>(p_source));
    case ImGuiDataType_Double:
      return static_cast<float>(*reinterpret_cast<double*>(p_source));
  }
  
  return 0.0f;
}

ImU64  FloatToScalar(ImGuiDataType data_type, float f_value)
{
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = static_cast<ImS8>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_U8:
    {
      ImU8 value = static_cast<ImU8>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_S16:
    {
      ImS16 value = static_cast<ImS16>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_U16:
    {
      ImU16 value = static_cast<ImU16>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_S32:
    {
      ImS32 value = static_cast<ImS32>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_U32:
    {
      ImU32 value = static_cast<ImU32>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_S64:
    {
      ImS64 value = static_cast<ImS64>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_U64:
    {
      ImU64 value = static_cast<ImU64>(f_value);
      return static_cast<ImU64>(value);
    }
    case ImGuiDataType_Float:
    {
      float value = f_value;
      return *reinterpret_cast<ImU64*>(&value);
    }
    case ImGuiDataType_Double:
    {
      double value = static_cast<double>(f_value);
      return *reinterpret_cast<ImU64*>(&value);
    }
  }
  
  return ImU64();
}


ImU64  SubScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result = 0;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = *reinterpret_cast<ImS8*>(p_a) - *static_cast<ImS8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 value = *reinterpret_cast<ImU8*>(p_a) - *static_cast<ImU8*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(p_a) - *static_cast<ImS16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 value = *reinterpret_cast<ImU16*>(p_a) - *static_cast<ImU16*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(p_a) - *static_cast<ImS32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 value = *reinterpret_cast<ImU32*>(p_a) - *static_cast<ImU32*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(p_a) - *static_cast<ImS64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 value = *reinterpret_cast<ImU64*>(p_a) - *static_cast<ImU64*>(p_b);
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(p_a) - *static_cast<float*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) - *static_cast<double*>(p_b);
      result = *reinterpret_cast<ImU64*>(&value);
    }
      break;
  }
  
  return result;
}

bool ImGuiExtensions::Slider2DScalar(const char *pLabel, ImGuiDataType data_type, void *p_valueX, void *p_valueY, void *p_minX, void *p_maxX, void *p_minY, void *p_maxY, const float fScale) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;
  
  //ImGuiContext& g = *GImGui;
  //const ImGuiStyle& style = g.Style;
  //const ImGuiID id = window->GetID(pLabel);
  
  ImGuiID const iID = ImGui::GetID(pLabel);
  
  ImVec2 const vSizeSubstract = ImGui::CalcTextSize(std::to_string(1.0f).c_str());
  
  float const vSizeFull = (ImGui::GetContentRegionAvail().x - vSizeSubstract.x) * fScale;
  ImVec2 const vSize(vSizeFull, vSizeFull);
  
  float const fHeightOffset = ImGui::GetTextLineHeight();
  ImVec2 const vHeightOffset(0.0f, fHeightOffset);
  
  ImVec2 vPos = ImGui::GetCursorScreenPos();
  ImRect oRect(vPos + vHeightOffset, vPos + vSize + vHeightOffset);
  
  ImGui::Text(pLabel);
  
  ImGui::PushID(iID);
  
  ImU32 const uFrameCol = ImGui::GetColorU32(ImGuiCol_FrameBg);
  
  ImVec2 const vOriginPos = ImGui::GetCursorScreenPos();
  ImGui::RenderFrame(oRect.Min, oRect.Max, uFrameCol, false, 0.0f);
  
  ImU64 s_delta_x = SubScalar(data_type, p_maxX, p_minX);
  ImU64 s_delta_y = SubScalar(data_type, p_maxY, p_minY);
  
  bool bModified = false;
  ImVec2 const vSecurity(15.0f, 15.0f);
  ImRect frame_bb = ImRect(oRect.Min - vSecurity, oRect.Max + vSecurity);
  //ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
  bool hovered;
  bool held;
  bool pressed = ImGui::ButtonBehavior(frame_bb, ImGui::GetID("##Zone"), &hovered, &held);
  if (hovered && held)
  {
    ImVec2 const vCursorPos = ImGui::GetMousePos() - oRect.Min;
    
    float fValueX = vCursorPos.x / (oRect.Max.x - oRect.Min.x) * ScalarToFloat(data_type, &s_delta_x) + ScalarToFloat(data_type, (ImU64*)p_minX);
    float fValueY = ScalarToFloat(data_type, &s_delta_y) - vCursorPos.y / (oRect.Max.y - oRect.Min.y) * ScalarToFloat(data_type, &s_delta_y) + ScalarToFloat(data_type, (ImU64*)p_minY);
    
    ImU64 s_value_x = FloatToScalar(data_type, fValueX);
    ImU64 s_value_y = FloatToScalar(data_type, fValueY);
    
    EqualScalar(data_type, (ImU64*)p_valueX, &s_value_x);
    EqualScalar(data_type, (ImU64*)p_valueY, &s_value_y);
    
    bModified = true;
  }
  //ImGui::PopItemFlag();
  
  ImU64 s_clamped_x = ClampScalar(data_type, p_valueX, p_minX, p_maxX);
  ImU64 s_clamped_y = ClampScalar(data_type, p_valueY, p_minY, p_maxY);
  EqualScalar(data_type, (ImU64*)p_valueX, &s_clamped_x);
  EqualScalar(data_type, (ImU64*)p_valueY, &s_clamped_y);
  
  float const fScaleX = (ScalarToFloat(data_type, (ImU64*)p_valueX) - ScalarToFloat(data_type, (ImU64*)p_minX)) / ScalarToFloat(data_type, &s_delta_x);
  float const fScaleY = 1.0f - (ScalarToFloat(data_type, (ImU64*)p_valueY) - ScalarToFloat(data_type, (ImU64*)p_minY)) / ScalarToFloat(data_type, &s_delta_y);
  
  constexpr float fCursorOff = 10.0f;
  float const fXLimit = fCursorOff / oRect.GetWidth();
  float const fYLimit = fCursorOff / oRect.GetHeight();
  
  ImVec2 const vCursorPos((oRect.Max.x - oRect.Min.x) * fScaleX + oRect.Min.x, (oRect.Max.y - oRect.Min.y) * fScaleY + oRect.Min.y);
  
  ImDrawList* pDrawList = ImGui::GetWindowDrawList();
  
  ImVec4 const vBlue(70.0f / 255.0f, 102.0f / 255.0f, 230.0f / 255.0f, 1.0f); // TODO: choose from style
  ImVec4 const vOrange(255.0f / 255.0f, 128.0f / 255.0f, 64.0f / 255.0f, 1.0f); // TODO: choose from style
  
  ImS32 const uBlue = ImGui::GetColorU32(vBlue);
  ImS32 const uOrange = ImGui::GetColorU32(vOrange);
  
  constexpr float fBorderThickness  = 2.0f;
  constexpr float fLineThickness    = 3.0f;
  constexpr float fHandleRadius    = 7.0f;
  constexpr float fHandleOffsetCoef  = 2.0f;
  
  // Cursor
  pDrawList->AddCircleFilled(vCursorPos, 5.0f, uBlue, 16);
  
  // Vertical Line
  if (fScaleY > 2.0f * fYLimit)
    pDrawList->AddLine(ImVec2(vCursorPos.x, oRect.Min.y + fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y - fCursorOff), uOrange, fLineThickness);
  if (fScaleY < 1.0f - 2.0f * fYLimit)
    pDrawList->AddLine(ImVec2(vCursorPos.x, oRect.Max.y - fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y + fCursorOff), uOrange, fLineThickness);
  
  // Horizontal Line
  if (fScaleX > 2.0f * fXLimit)
    pDrawList->AddLine(ImVec2(oRect.Min.x + fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x - fCursorOff, vCursorPos.y), uOrange, fLineThickness);
  if (fScaleX < 1.0f - 2.0f * fYLimit)
    pDrawList->AddLine(ImVec2(oRect.Max.x - fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x + fCursorOff, vCursorPos.y), uOrange, fLineThickness);
  
  std::string formatX = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
  std::string formatY = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
  
  if (IsPositiveScalar(data_type, (ImU64*)p_valueX))
  {
    formatX = " " + formatX;
  }
  if (IsPositiveScalar(data_type, (ImU64*)p_valueY))
  {
    formatY = " " + formatY;
  }
  
  char pBufferX[64];
  char pBufferY[64];
  /*const char* value_buf_end_x = pBufferX + */ImGui::DataTypeFormatString(pBufferX, IM_ARRAYSIZE(pBufferX), data_type, p_valueX, formatX.c_str());
  /*const char* value_buf_end_y = pBufferX + */ImGui::DataTypeFormatString(pBufferY, IM_ARRAYSIZE(pBufferY), data_type, p_valueY, formatY.c_str());
  
  ImU32 const uTextCol = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
  
  ImGui::SetWindowFontScale(0.75f);
  
  ImVec2 const vXSize = ImGui::CalcTextSize(pBufferX);
  ImVec2 const vYSize = ImGui::CalcTextSize(pBufferY);
  
  ImVec2 const vHandlePosX = ImVec2(vCursorPos.x, oRect.Max.y + vYSize.x * 0.5f);
  ImVec2 const vHandlePosY = ImVec2(oRect.Max.x + fHandleOffsetCoef * fCursorOff + vYSize.x, vCursorPos.y);
  
  ImRect handle_x_bb = ImRect(vHandlePosX - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosX + ImVec2(fHandleRadius, fHandleRadius) + vSecurity);
  ImRect handle_y_bb = ImRect(vHandlePosY - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosY + ImVec2(fHandleRadius, fHandleRadius) + vSecurity);
  pressed = ImGui::ButtonBehavior(handle_x_bb, ImGui::GetID("##HandleX"), &hovered, &held);
  if (hovered && held)
  {
    ImVec2 const vCursorPosLocal = ImGui::GetMousePos() - oRect.Min;
    
    //*fValueX = vCursorPos.x / (oRect.Max.x - oRect.Min.x) * fDeltaX + fMinX;
    float fValueX = vCursorPosLocal.x / (oRect.Max.x - oRect.Min.x) * ScalarToFloat(data_type, &s_delta_x) + ScalarToFloat(data_type, (ImU64*)p_minX);
    ImU64 s_value_x = FloatToScalar(data_type, fValueX);
    EqualScalar(data_type, (ImU64*)p_valueX, &s_value_x);
    
    bModified = true;
  }
  pressed = ImGui::ButtonBehavior(handle_y_bb, ImGui::GetID("##HandleX"), &hovered, &held);
  if (hovered && held)
  {
    ImVec2 const vCursorPosLocal = ImGui::GetMousePos() - oRect.Min;
    
    //*fValueY = fDeltaY - vCursorPos.y / (oRect.Max.y - oRect.Min.y) * fDeltaY + fMinY;
    float fValueY = ScalarToFloat(data_type, &s_delta_y) - vCursorPosLocal.y / (oRect.Max.y - oRect.Min.y) * ScalarToFloat(data_type, &s_delta_y) + ScalarToFloat(data_type, (ImU64*)p_minY);
    ImU64 s_value_y = FloatToScalar(data_type, fValueY);
    EqualScalar(data_type, (ImU64*)p_valueY, &s_value_y);
    
    bModified = true;
  }
  
  pDrawList->AddText(
                     ImVec2(
                            ImMin(ImMax(vCursorPos.x - vXSize.x * 0.5f, oRect.Min.x), oRect.Min.x + vSize.x - vXSize.x),
                            oRect.Max.y + fCursorOff),
                     uTextCol,
                     pBufferX);
  pDrawList->AddText(
                     ImVec2(oRect.Max.x + fCursorOff, ImMin(ImMax(vCursorPos.y - vYSize.y * 0.5f, oRect.Min.y),
                                                            oRect.Min.y + vSize.y - vYSize.y)),
                     uTextCol,
                     pBufferY);
  ImGui::SetWindowFontScale(1.0f);
  
  // Borders::Right
  pDrawList->AddCircleFilled(ImVec2(oRect.Max.x, vCursorPos.y), 2.0f, uOrange, 3);
  // Handle Right::Y
  pDrawList->AddNgonFilled(ImVec2(oRect.Max.x + fHandleOffsetCoef * fCursorOff + vYSize.x, vCursorPos.y), fHandleRadius, uBlue, 4);
  if (fScaleY > fYLimit)
    pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Min.y), ImVec2(oRect.Max.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
  if (fScaleY < 1.0f - fYLimit)
    pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Max.y), ImVec2(oRect.Max.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
  // Borders::Top
  pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oRect.Min.y), 2.0f, uOrange, 3);
  if (fScaleX > fXLimit)
    pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Min.y), ImVec2(vCursorPos.x - fCursorOff, oRect.Min.y), uBlue, fBorderThickness);
  if (fScaleX < 1.0f - fXLimit)
    pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Min.y), ImVec2(vCursorPos.x + fCursorOff, oRect.Min.y), uBlue, fBorderThickness);
  // Borders::Left
  pDrawList->AddCircleFilled(ImVec2(oRect.Min.x, vCursorPos.y), 2.0f, uOrange, 3);
  if (fScaleY > fYLimit)
    pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Min.y), ImVec2(oRect.Min.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
  if (fScaleY < 1.0f - fYLimit)
    pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Max.y), ImVec2(oRect.Min.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
  // Borders::Bottom
  pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oRect.Max.y), 2.0f, uOrange, 3);
  // Handle Bottom::X
  pDrawList->AddNgonFilled(ImVec2(vCursorPos.x, oRect.Max.y + vXSize.y * 2.0f), fHandleRadius, uBlue, 4);
  if (fScaleX > fXLimit)
    pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Max.y), ImVec2(vCursorPos.x - fCursorOff, oRect.Max.y), uBlue, fBorderThickness);
  if (fScaleX < 1.0f - fXLimit)
    pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Max.y), ImVec2(vCursorPos.x + fCursorOff, oRect.Max.y), uBlue, fBorderThickness);
  
  ImGui::PopID();
  
  ImGui::Dummy(vHeightOffset);
  ImGui::Dummy(vHeightOffset);
  ImGui::Dummy(vSize);
  
  return bModified;
}


bool ImGuiExtensions::Slider2DFloat(const char *pLabel, float *pValueX, float *pValueY, float minX, float maxX, float minY, float maxY, const float fScale) {
  return Slider2DScalar(pLabel, ImGuiDataType_Float, pValueX, pValueY, &minX, &maxX, &minY, &maxY, fScale);
}



namespace ImGui
{

static inline float RoundScalarWithFormatFloat(const char* format, ImGuiDataType data_type, float v)
{
  return RoundScalarWithFormatT<float>(format, data_type, v);
}


static inline float SliderCalcRatioFromValueFloat(ImGuiDataType data_type, float v, float v_min, float v_max, float power, float linear_zero_pos)
{
  return ScaleRatioFromValueT<float, float, float>(data_type, v, v_min, v_max, false, power, linear_zero_pos);
}


// ~80% common code with ImGui::SliderBehavior
bool RangeSliderBehavior(const ImRect& frame_bb, ImGuiID id, float* v1, float* v2, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags)
{
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = GetCurrentWindow();
  const ImGuiStyle& style = g.Style;
  
  // Draw frame
  RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
  
  const bool is_non_linear = (power < 1.0f-0.00001f) || (power > 1.0f+0.00001f);
  const bool is_horizontal = (flags & ImGuiSliderFlags_Vertical) == 0;
  
  const float grab_padding = 2.0f;
  const float slider_sz = is_horizontal ? (frame_bb.GetWidth() - grab_padding * 2.0f) : (frame_bb.GetHeight() - grab_padding * 2.0f);
  float grab_sz;
  if (decimal_precision > 0)
    grab_sz = ImMin(style.GrabMinSize, slider_sz);
  else
    grab_sz = ImMin(ImMax(1.0f * (slider_sz / ((v_min < v_max ? v_max - v_min : v_min - v_max) + 1.0f)), style.GrabMinSize), slider_sz);  // Integer sliders, if possible have the grab size represent 1 unit
  const float slider_usable_sz = slider_sz - grab_sz;
  const float slider_usable_pos_min = (is_horizontal ? frame_bb.Min.x : frame_bb.Min.y) + grab_padding + grab_sz*0.5f;
  const float slider_usable_pos_max = (is_horizontal ? frame_bb.Max.x : frame_bb.Max.y) - grab_padding - grab_sz*0.5f;
  
  // For logarithmic sliders that cross over sign boundary we want the exponential increase to be symmetric around 0.0f
  float linear_zero_pos = 0.0f;   // 0.0->1.0f
  if (v_min * v_max < 0.0f)
  {
    // Different sign
    const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f/power);
    const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f/power);
    linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0+linear_dist_max_to_0);
  }
  else
  {
    // Same sign
    linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
  }
  
  // Process clicking on the slider
  bool value_changed = false;
  if (g.ActiveId == id)
  {
    if (g.IO.MouseDown[0])
    {
      const float mouse_abs_pos = is_horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
      float clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
      if (!is_horizontal)
        clicked_t = 1.0f - clicked_t;
      
      float new_value;
      if (is_non_linear)
      {
        // Account for logarithmic scale on both sides of the zero
        if (clicked_t < linear_zero_pos)
        {
          // Negative: rescale to the negative range before powering
          float a = 1.0f - (clicked_t / linear_zero_pos);
          a = powf(a, power);
          new_value = ImLerp(ImMin(v_max,0.0f), v_min, a);
        }
        else
        {
          // Positive: rescale to the positive range before powering
          float a;
          if (fabsf(linear_zero_pos - 1.0f) > 1.e-6f)
            a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
          else
            a = clicked_t;
          a = powf(a, power);
          new_value = ImLerp(ImMax(v_min,0.0f), v_max, a);
        }
      }
      else
      {
        // Linear slider
        new_value = ImLerp(v_min, v_max, clicked_t);
      }
      
      char fmt[64];
      snprintf(fmt, 64, "%%.%df", decimal_precision);
      
      // Round past decimal precision
      new_value = RoundScalarWithFormatT(fmt, ImGuiDataType_Float, new_value);
      if (*v1 != new_value || *v2 != new_value)
      {
        if (fabsf(*v1 - new_value) < fabsf(*v2 - new_value))
        {
          *v1 = new_value;
        }
        else
        {
          *v2 = new_value;
        }
        value_changed = true;
      }
    }
    else
    {
      ClearActiveID();
    }
  }
  
  // Calculate slider grab positioning
  float grab_t = SliderCalcRatioFromValueFloat(ImGuiDataType_Float, *v1, v_min, v_max, power, linear_zero_pos);
  
  // Draw
  if (!is_horizontal)
    grab_t = 1.0f - grab_t;
  float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
  ImRect grab_bb1;
  if (is_horizontal)
    grab_bb1 = ImRect(ImVec2(grab_pos - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
  else
    grab_bb1 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f));
  window->DrawList->AddRectFilled(grab_bb1.Min, grab_bb1.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  
  // Calculate slider grab positioning
  grab_t = SliderCalcRatioFromValueFloat(ImGuiDataType_Float, *v2, v_min, v_max, power, linear_zero_pos);
  
  // Draw
  if (!is_horizontal)
    grab_t = 1.0f - grab_t;
  grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
  ImRect grab_bb2;
  if (is_horizontal)
    grab_bb2 = ImRect(ImVec2(grab_pos - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
  else
    grab_bb2 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f));
  window->DrawList->AddRectFilled(grab_bb2.Min, grab_bb2.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  
  ImRect connector(grab_bb1.Min, grab_bb2.Max);
  connector.Min.x += grab_sz;
  connector.Min.y += grab_sz*0.3f;
  connector.Max.x -= grab_sz;
  connector.Max.y -= grab_sz*0.3f;
  
  window->DrawList->AddRectFilled(connector.Min, connector.Max, GetColorU32(ImGuiCol_SliderGrab), style.GrabRounding);
  
  return value_changed;
}

// ~95% common code with ImGui::SliderFloat
bool RangeSliderFloat(const char* label, float* v1, float* v2, float v_min, float v_max, const char* display_format, float power)
{
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return false;
  
  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);
  const float w = CalcItemWidth();
  
  const ImVec2 label_size = CalcTextSize(label, NULL, true);
  const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
  const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
  
  // NB- we don't call ItemSize() yet because we may turn into a text edit box below
  if (!ItemAdd(total_bb, id))
  {
    ItemSize(total_bb, style.FramePadding.y);
    return false;
  }
  
  const bool hovered = ItemHoverable(frame_bb, id);
  if (hovered)
    SetHoveredID(id);
  
  if (!display_format)
    display_format = "(%.3f, %.3f)";
  int decimal_precision = ImParseFormatPrecision(display_format, 3);
  
  // Tabbing or CTRL-clicking on Slider turns it into an input box
  bool start_text_input = false;
  if (hovered && g.IO.MouseClicked[0])
  {
    SetActiveID(id, window);
    FocusWindow(window);
    
    if (g.IO.KeyCtrl)
    {
      start_text_input = true;
      g.TempInputId = 0;
    }
  }
  
  if (start_text_input || (g.ActiveId == id && g.TempInputId == id))
  {
    char fmt[64];
    snprintf(fmt, 64, "%%.%df", decimal_precision);
    return TempInputScalar(frame_bb, id, label, ImGuiDataType_Float, v1, fmt);
  }
  
  ItemSize(total_bb, style.FramePadding.y);
  
  // Actual slider behavior + render grab
  const bool value_changed = RangeSliderBehavior(frame_bb, id, v1, v2, v_min, v_max, power, decimal_precision, 0);
  
  // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
  char value_buf[64];
  const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v1, *v2);
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.5f));
  
  if (label_size.x > 0.0f)
    RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
  
  return value_changed;
}

}

void ImGui::BeginGroupPanel(const char* name, const ImVec2& size)
{
  ImGui::BeginGroup();
  
  auto cursorPos = ImGui::GetCursorScreenPos();
  auto itemSpacing = ImGui::GetStyle().ItemSpacing;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
  
  auto frameHeight = ImGui::GetFrameHeight();
  ImGui::BeginGroup();
  
  ImVec2 effectiveSize = size;
  if (size.x < 0.0f)
    effectiveSize.x = ImGui::GetContentRegionAvail().x;
  else
    effectiveSize.x = size.x;
  ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));
  
  ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
  ImGui::SameLine(0.0f, 0.0f);
  ImGui::BeginGroup();
  ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
  ImGui::SameLine(0.0f, 0.0f);
  ImGui::SameLine(0.0f, 0.0f);
  ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
  ImGui::BeginGroup();
  
  ImGui::PopStyleVar(2);
  
  ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
  ImGui::GetCurrentWindow()->Size.x                   -= frameHeight;
  
  ImGui::PushItemWidth(effectiveSize.x - frameHeight);
}

bool ImGuiExtensions::RangeSelect2D(const char *pLabel, float *pCurMinX, float *pCurMinY, float *pCurMaxX, float *pCurMaxY, const float fBoundMinX, const float fBoundMinY, const float fBoundMaxX, const float fBoundMaxY, const float fScale) {
  float& fCurMinX = *pCurMinX;
  float& fCurMinY = *pCurMinY;
  float& fCurMaxX = *pCurMaxX;
  float& fCurMaxY = *pCurMaxY;

  ImGuiID const iID = ImGui::GetID(pLabel);
  ImGui::PushID(iID);

  float const vSizeFull = (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("1.0").x) * fScale;
  ImVec2 const vSize(vSizeFull, vSizeFull);

  ImVec2 vPos = ImGui::GetCursorScreenPos();
  ImRect oRect(vPos, vPos + vSize);
  CommonViews::Spacing(vSizeFull);

  // Convert current min and max from data space to screen space
  float const fScaleMinX = Rescale01(fCurMinX, fBoundMinX, fBoundMaxX);
  float const fScaleMinY = Rescale01(fCurMinY, fBoundMinY, fBoundMaxY);
  float const fScaleMaxX = Rescale01(fCurMaxX, fBoundMinX, fBoundMaxX);
  float const fScaleMaxY = Rescale01(fCurMaxY, fBoundMinY, fBoundMaxY);

  float const fRegMinX = ImLerp(oRect.Min.x, oRect.Max.x, fScaleMinX);
  float const fRegMinY = ImLerp(oRect.Min.y, oRect.Max.y, fScaleMinY);
  float const fRegMaxX = ImLerp(oRect.Min.x, oRect.Max.x, fScaleMaxX);
  float const fRegMaxY = ImLerp(oRect.Min.y, oRect.Max.y, fScaleMaxY);

  ImRect oRegionRect(fRegMinX, fRegMinY, fRegMaxX, fRegMaxY);
	
  bool bModified = false;

  ImVec2 vHandleSize(10.0f, 10.0f);  // Handle size for visuals
  ImU32 handleColor = ImGui::GetColorU32(ImGuiCol_Button);  // Color of the handles

  // Min handle interaction
  ImRect minHandleRect(fRegMinX - vHandleSize.x / 2, fRegMinY - vHandleSize.y / 2, fRegMinX + vHandleSize.x / 2, fRegMinY + vHandleSize.y / 2);
  bool hovered, held;
  bool pressed = ImGui::ButtonBehavior(minHandleRect, ImGui::GetID("##minHandle"), &hovered, &held);
  if (held) {
      ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
      ImVec2 newMin = { ImClamp(fCurMinX + mouseDelta.x, fBoundMinX, fCurMaxX), ImClamp(fCurMinY + mouseDelta.y, fBoundMinY, fCurMaxY) };
      fCurMinX = newMin.x;
      fCurMinY = newMin.y;
      bModified = true;
  }

  // Max handle interaction
  ImRect maxHandleRect(fRegMaxX - vHandleSize.x / 2, fRegMaxY - vHandleSize.y / 2, fRegMaxX + vHandleSize.x / 2, fRegMaxY + vHandleSize.y / 2);
  pressed = ImGui::ButtonBehavior(maxHandleRect, ImGui::GetID("##maxHandle"), &hovered, &held);
  if (held) {
      ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
      ImVec2 newMax = { ImClamp(fCurMaxX + mouseDelta.x, fCurMinX, fBoundMaxX), ImClamp(fCurMaxY + mouseDelta.y, fCurMinY, fBoundMaxY) };
      fCurMaxX = newMax.x;
      fCurMaxY = newMax.y;
      bModified = true;
  }

  // Draw the frame and handles
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRect(oRect.Min, oRect.Max, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
  draw_list->AddRectFilled(minHandleRect.Min, minHandleRect.Max, handleColor);
  draw_list->AddRectFilled(maxHandleRect.Min, maxHandleRect.Max, handleColor);

  ImGui::PopID();

  return bModified;
}



void ImGui::EndGroupPanel()
{
  ImGui::PopItemWidth();
  
  auto itemSpacing = ImGui::GetStyle().ItemSpacing;
  
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
  
  auto frameHeight = ImGui::GetFrameHeight();
  
  ImGui::EndGroup();
  
  ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 45), 4.0f);
  
  ImGui::EndGroup();
  
  ImGui::SameLine(0.0f, 0.0f);
  ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
  ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));
  
  ImGui::EndGroup();
  
  auto itemMin = ImGui::GetItemRectMin();
  auto itemMax = ImGui::GetItemRectMax();
  //ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);
  
  ImVec2 halfFrame = ImVec2(frameHeight * 0.25f * 0.5f, frameHeight * 0.5f);
  ImGui::GetWindowDrawList()->AddRect(
                                      itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f),
                                      ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
                                      halfFrame.x);
  
  ImGui::PopStyleVar(2);
  
  ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
  ImGui::GetCurrentWindow()->Size.x                   += frameHeight;
  
  ImGui::Dummy(ImVec2(0.0f, 0.0f));
  
  ImGui::EndGroup();
}
