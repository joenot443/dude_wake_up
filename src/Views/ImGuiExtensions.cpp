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
#include <string> // Required for std::to_string
#include <algorithm> // Required for std::min/std::max

float Rescale01(float const x, float const min, float const max)
{
  if ((max - min) == 0.0f) return 0.0f;
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
    case ImGuiDataType_S16:
    {
      ImS16 value = *reinterpret_cast<ImS16*>(src);
      return value < 0;
    }
    case ImGuiDataType_S32:
    {
      ImS32 value = *reinterpret_cast<ImS32*>(src);
      return value < 0l;
    }
    case ImGuiDataType_S64:
    {
      ImS64 value = *reinterpret_cast<ImS64*>(src);
      return value < 0ll;
    }
    case ImGuiDataType_U8:
    case ImGuiDataType_U16:
    case ImGuiDataType_U32:
    case ImGuiDataType_U64:
    {
      return false;
    }
    case ImGuiDataType_Float:
    {
      float value = *reinterpret_cast<float*>(src);
      return value < 0.0f;
    }
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(src);
      return value < 0.0f;
    }
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
      *reinterpret_cast<ImU32*>(p_target) = *reinterpret_cast<ImU32*>(p_source);
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
  ImU64 result = 0;
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
      memcpy(&result, &value, sizeof(float));
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) * *static_cast<double*>(p_b);
      memcpy(&result, &value, sizeof(double));
    }
      break;
  }
  
  return result;
}

ImU64  DivScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result = 0;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 b_val = *static_cast<ImS8*>(p_b);
      if (b_val == 0) { /* Handle error or return specific value */ break; }
      ImS8 value = *reinterpret_cast<ImS8*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U8:
    {
      ImU8 b_val = *static_cast<ImU8*>(p_b);
      if (b_val == 0) break;
      ImU8 value = *reinterpret_cast<ImU8*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S16:
    {
      ImS16 b_val = *static_cast<ImS16*>(p_b);
      if (b_val == 0) break;
      ImS16 value = *reinterpret_cast<ImS16*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U16:
    {
      ImU16 b_val = *static_cast<ImU16*>(p_b);
      if (b_val == 0) break;
      ImU16 value = *reinterpret_cast<ImU16*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S32:
    {
      ImS32 b_val = *static_cast<ImS32*>(p_b);
      if (b_val == 0) break;
      ImS32 value = *reinterpret_cast<ImS32*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U32:
    {
      ImU32 b_val = *static_cast<ImU32*>(p_b);
      if (b_val == 0) break;
      ImU32 value = *reinterpret_cast<ImU32*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_S64:
    {
      ImS64 b_val = *static_cast<ImS64*>(p_b);
      if (b_val == 0) break;
      ImS64 value = *reinterpret_cast<ImS64*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_U64:
    {
      ImU64 b_val = *static_cast<ImU64*>(p_b);
      if (b_val == 0) break;
      ImU64 value = *reinterpret_cast<ImU64*>(p_a) / b_val;
      result = static_cast<ImU64>(value);
    }
      break;
    case ImGuiDataType_Float:
    {
      float b_val = *static_cast<float*>(p_b);
      if (b_val == 0.0f) break;
      float value = *reinterpret_cast<float*>(p_a) / b_val;
      memcpy(&result, &value, sizeof(float));
    }
      break;
    case ImGuiDataType_Double:
    {
      double b_val = *static_cast<double*>(p_b);
      if (b_val == 0.0) break;
      double value = *reinterpret_cast<double*>(p_a) / b_val;
      memcpy(&result, &value, sizeof(double));
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
      memcpy(&result, &value, sizeof(float));
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = ImClamp(*reinterpret_cast<double*>(p_value), *static_cast<double*>(p_min), *static_cast<double*>(p_max));
      memcpy(&result, &value, sizeof(double));
    }
      break;
  }
  
  return result;
}


ImU64  AddScalar(ImGuiDataType data_type, void* p_a, void* p_b)
{
  ImU64 result = 0;
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
      memcpy(&result, &value, sizeof(float));
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) + *static_cast<double*>(p_b);
      memcpy(&result, &value, sizeof(double));
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
  ImU64 result = 0;
  switch (data_type)
  {
    case ImGuiDataType_S8:
    {
      ImS8 value = static_cast<ImS8>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_U8:
    {
      ImU8 value = static_cast<ImU8>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_S16:
    {
      ImS16 value = static_cast<ImS16>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_U16:
    {
      ImU16 value = static_cast<ImU16>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_S32:
    {
      ImS32 value = static_cast<ImS32>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_U32:
    {
      ImU32 value = static_cast<ImU32>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_S64:
    {
      ImS64 value = static_cast<ImS64>(f_value);
      result = static_cast<ImU64>(value);
    }
    case ImGuiDataType_U64:
    {
      ImU64 value = static_cast<ImU64>(f_value);
      result = value;
    }
    case ImGuiDataType_Float:
    {
      float value = f_value;
      memcpy(&result, &value, sizeof(float));
    }
    case ImGuiDataType_Double:
    {
      double value = static_cast<double>(f_value);
      memcpy(&result, &value, sizeof(double));
    }
  }
  return result;
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
      memcpy(&result, &value, sizeof(float));
    }
      break;
    case ImGuiDataType_Double:
    {
      double value = *reinterpret_cast<double*>(p_a) - *static_cast<double*>(p_b);
      memcpy(&result, &value, sizeof(double));
    }
      break;
  }
  
  return result;
}


bool ImGuiExtensions::Slider2DScalar(char const* pLabel, ImGuiDataType data_type, void* p_valueX, void* p_valueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY, float const aspectRatio) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;
  
  ImGuiContext &g = *ImGui::GetCurrentContext();
  const ImGuiStyle &style = g.Style;
  
  ImGuiID id = window->GetID(pLabel);
  ImGui::PushID(id);
  
  // Determine control size
  ImVec2 available = ImGui::GetContentRegionAvail();
  float controlWidth = available.x;;
  controlWidth = ImMax(controlWidth, ImGui::GetFontSize() * 4.0f);
  ImVec2 controlSize(controlWidth, controlWidth * aspectRatio);
  
  ImVec2 cursorPos = ImGui::GetCursorScreenPos();
  ImRect frame_bb(cursorPos, ImVec2(cursorPos.x + controlSize.x, cursorPos.y + controlSize.y));
  
  ImGui::ItemSize(frame_bb);
  if (!ImGui::ItemAdd(frame_bb, id, &frame_bb)) {
    ImGui::PopID();
    return false;
  }
  
  bool valueChanged = false;
  bool hovered = ImGui::IsItemHovered();
  
  // Interaction logic
  bool pressed = ImGui::ButtonBehavior(frame_bb, id, nullptr, nullptr, ImGuiButtonFlags_PressedOnClick);
  
  if (pressed || (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
    ImVec2 mousePos = ImGui::GetMousePos();
    float localX = (mousePos.x - frame_bb.Min.x) / frame_bb.GetWidth();
    float localY = (mousePos.y - frame_bb.Min.y) / frame_bb.GetHeight();
    
    localX = ImClamp(localX, 0.0f, 1.0f);
    localY = ImClamp(localY, 0.0f, 1.0f);
    
    // Calculate new values
    float minX = *(float*)p_minX, maxX = *(float*)p_maxX;
    float minY = *(float*)p_minY, maxY = *(float*)p_maxY;
    
    float newX = minX + localX * (maxX - minX);
    float newY = minY + localY * (maxY - minY); // Y is 0 at top, maxY at bottom
    
    *(float*)p_valueX = newX;
    *(float*)p_valueY = newY;
    
    valueChanged = true;
    ImGui::MarkItemEdited(id);
  }
  
  // Draw visuals
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImU32 col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
  drawList->AddRectFilled(frame_bb.Min, frame_bb.Max, col, style.FrameRounding);
  
  ImVec2 cursorPosDraw = ImVec2(
                                frame_bb.Min.x + ((*(float*)p_valueX - *(float*)p_minX) / (*(float*)p_maxX - *(float*)p_minX)) * frame_bb.GetWidth(),
                                frame_bb.Min.y + ((*(float*)p_valueY - *(float*)p_minY) / (*(float*)p_maxY - *(float*)p_minY)) * frame_bb.GetHeight()
                                );
  
  drawList->AddCircleFilled(cursorPosDraw, style.GrabMinSize * 0.5f, ImGui::GetColorU32(ImGuiCol_SliderGrabActive));
  
  ImGui::PopID();
  
  return valueChanged;
}



bool ImGuiExtensions::Slider2DFloat(const char *pLabel, float *pValueX, float *pValueY, float minX, float maxX, float minY, float maxY, const float aspectRatio) {
  return Slider2DScalar(pLabel, ImGuiDataType_Float, pValueX, pValueY, &minX, &maxX, &minY, &maxY, aspectRatio);
}



namespace ImGui
{

static inline float RoundScalarWithFormatFloat(const char* format, ImGuiDataType data_type, float v)
{
  return ImGui::RoundScalarWithFormatT<float>(format, data_type, v);
}


static inline float SliderCalcRatioFromValueFloat(ImGuiDataType data_type, float v, float v_min, float v_max, float power, float linear_zero_pos)
{
  return ImGui::ScaleRatioFromValueT<float, float, float>(data_type, v, v_min, v_max, false, power, linear_zero_pos);
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
  
  float linear_zero_pos = 0.0f;
  if (v_min * v_max < 0.0f)
  {
    const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f/power);
    const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f/power);
    linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0+linear_dist_max_to_0);
  }
  else
  {
    linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
  }
  
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
        if (clicked_t < linear_zero_pos)
        {
          float a = 1.0f - (clicked_t / linear_zero_pos);
          a = powf(a, power);
          new_value = ImLerp(ImMin(v_max,0.0f), v_min, a);
        }
        else
        {
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
        new_value = ImLerp(v_min, v_max, clicked_t);
      }
      
      char fmt[64];
      snprintf(fmt, 64, "%%.%df", decimal_precision);
      
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
        MarkItemEdited(id);
      }
    }
    else
    {
      ClearActiveID();
    }
  }
  
  float grab_t_v1 = SliderCalcRatioFromValueFloat(ImGuiDataType_Float, *v1, v_min, v_max, power, linear_zero_pos);
  if (!is_horizontal)
    grab_t_v1 = 1.0f - grab_t_v1;
  float grab_pos_v1 = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t_v1);
  ImRect grab_bb1;
  if (is_horizontal)
    grab_bb1 = ImRect(ImVec2(grab_pos_v1 - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos_v1 + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
  else
    grab_bb1 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos_v1 - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos_v1 + grab_sz*0.5f));
  window->DrawList->AddRectFilled(grab_bb1.Min, grab_bb1.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  
  float grab_t_v2 = SliderCalcRatioFromValueFloat(ImGuiDataType_Float, *v2, v_min, v_max, power, linear_zero_pos);
  if (!is_horizontal)
    grab_t_v2 = 1.0f - grab_t_v2;
  float grab_pos_v2 = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t_v2);
  ImRect grab_bb2;
  if (is_horizontal)
    grab_bb2 = ImRect(ImVec2(grab_pos_v2 - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos_v2 + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
  else
    grab_bb2 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos_v2 - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos_v2 + grab_sz*0.5f));
  window->DrawList->AddRectFilled(grab_bb2.Min, grab_bb2.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  
  ImRect connector(ImMin(grab_bb1.Min, grab_bb2.Min), ImMax(grab_bb1.Max, grab_bb2.Max));
  if (is_horizontal) {
    connector.Min.y += grab_sz * 0.3f;
    connector.Max.y -= grab_sz * 0.3f;
  } else {
    connector.Min.x += grab_sz * 0.3f;
    connector.Max.x -= grab_sz * 0.3f;
  }
  window->DrawList->AddRectFilled(connector.Min, connector.Max, GetColorU32(ImGuiCol_SliderGrab), style.GrabRounding);
  
  return value_changed;
}

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
  
  const bool hovered = ItemHoverable(frame_bb, id, 0);
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
  ImGui::TextUnformatted(name);
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

  ImGuiWindow* window = ImGui::GetCurrentWindow();
  if (window->SkipItems) return false;
  
  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(pLabel);
  ImGui::PushID(id);

  float available_width = ImGui::GetContentRegionAvail().x;
  float control_size_full = available_width * fScale;
  control_size_full = std::max(control_size_full, ImGui::GetFontSize() * 5.0f);
  
  ImVec2 control_size(control_size_full, control_size_full);

  ImVec2 screen_pos = ImGui::GetCursorScreenPos();
  ImRect control_bb(screen_pos, ImVec2(screen_pos.x + control_size.x, screen_pos.y + control_size.y));
  
  ImGui::ItemSize(control_bb, style.FramePadding.y);
  if (!ImGui::ItemAdd(control_bb, id, &control_bb)) {
    ImGui::PopID();
    return false;
  }
  
  ImGui::RenderFrame(control_bb.Min, control_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

  bool bModified = false;

  float norm_min_x = Rescale01(fCurMinX, fBoundMinX, fBoundMaxX);
  float norm_min_y = Rescale01(fCurMinY, fBoundMinY, fBoundMaxY);
  float norm_max_x = Rescale01(fCurMaxX, fBoundMinX, fBoundMaxX);
  float norm_max_y = Rescale01(fCurMaxY, fBoundMinY, fBoundMaxY);

  ImRect selected_region_bb(ImVec2(control_bb.Min.x + norm_min_x * control_bb.GetWidth(), control_bb.Min.y + (1.0f - norm_max_y) * control_bb.GetHeight()), ImVec2(control_bb.Min.x + norm_max_x * control_bb.GetWidth(), control_bb.Min.y + (1.0f - norm_min_y) * control_bb.GetHeight()));

  if (selected_region_bb.Min.x > selected_region_bb.Max.x) { std::swap(selected_region_bb.Min.x, selected_region_bb.Max.x); }
  if (selected_region_bb.Min.y > selected_region_bb.Max.y) { std::swap(selected_region_bb.Min.y, selected_region_bb.Max.y); }

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(selected_region_bb.Min, selected_region_bb.Max, ImGui::GetColorU32(ImGuiCol_ButtonActive, 0.5f), style.FrameRounding);
  draw_list->AddRect(selected_region_bb.Min, selected_region_bb.Max, ImGui::GetColorU32(ImGuiCol_ButtonActive), style.FrameRounding);

  bModified = true;

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
  
  ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 0, 0, 64), 4.0f);
  
  ImGui::EndGroup();
  
  ImGui::SameLine(0.0f, 0.0f);
  ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
  ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));
  
  ImGui::EndGroup();
  
  auto itemMin = ImGui::GetItemRectMin();
  auto itemMax = ImGui::GetItemRectMax();
  
  ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight * 0.5f);
  ImGui::GetWindowDrawList()->AddRect(
                                      itemMin + ImVec2(halfFrame.x*0.5f, halfFrame.y*0.5f), itemMax - ImVec2(halfFrame.x*0.5f, halfFrame.y*0.5f),
                                      ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
                                      4.0f);
  
  ImGui::PopStyleVar(2);
  
  ImGui::Dummy(ImVec2(0.0f, itemSpacing.y));
}
