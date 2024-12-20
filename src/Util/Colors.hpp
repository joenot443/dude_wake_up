//
//  Colors.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2/17/23.
//

#ifndef Colors_h
#define Colors_h

#include <imgui.h>
#include "ShaderType.hpp"


// Redesign Colors

struct Colors {
  constexpr static ImColor NodeBackgroundColor = ImColor(32, 32, 35);
  constexpr static ImColor NodeBorderColor = ImColor(86, 86, 89);
  constexpr static ImColor NodeInnerRectBackgroundColor = IM_COL32(11, 11, 12, 255);
  
  constexpr static ImColor SecondaryDark = IM_COL32(44, 44, 48, 255);
  constexpr static ImColor SecondaryDark200 = IM_COL32(65, 65, 69, 255);
  constexpr static ImColor Secondary300 = IM_COL32(114, 114, 116, 255);
  constexpr static ImColor Secondary200 = IM_COL32(144, 144, 146, 255);
  

  constexpr static ImColor PinActive = IM_COL32(181, 81, 38, 255);
  constexpr static ImColor PinActiveSecondary = IM_COL32(181, 81, 38, 51);
  constexpr static ImColor PinActiveTertiary = IM_COL32(181, 81, 38, 25);

  // rgba(30, 30, 32, 1)
  constexpr static ImColor ButtonSelected = IM_COL32(50, 50, 52, 255);
  constexpr static ImColor ButtonSelectedHovered = IM_COL32(60, 60, 62, 255);
};
                                             
static ImColor SourceNodeColor = ImColor(174, 32, 18);
static ImColor ShaderNodeColor = ImColor(0, 95, 115);
static ImColor AuxNodeColor = ImColor(202, 103, 2);
static ImColor MaskNodeColor = ImColor(54, 78, 2);
static ImColor WarningColor = ImColor(233, 196, 106);
static ImColor GoodColor = ImColor(46, 204, 113);

// Whites

static ImColor White50 = ImColor(128, 128, 128);
static ImColor White60 = ImColor(153, 153, 153);
static ImColor White70 = ImColor(179, 179, 179);
static ImColor White80 = ImColor(204, 204, 204);
static ImColor White90 = ImColor(230, 230, 230);
static ImColor White100 = ImColor(255, 255, 255);

static std::vector<ImColor> colors = {
  ImColor(231, 76, 60), // Red
  ImColor(230, 126, 34), // Orange
  ImColor(241, 196, 15), // Yellow
  ImColor(46, 204, 113), // Bright green
  ImColor(26, 188, 156), // Aqua-green
  ImColor(52, 152, 219), // Sky blue
  ImColor(155, 89, 182), // Purple
  ImColor(52, 73, 94), // Dark gray-blue
  ImColor(236, 240, 241), // Light gray
  ImColor(149, 165, 166) // Medium gray
};

static std::vector<ImColor> nodeColors = {
    ImColor(22, 160, 133),
    ImColor(39, 174, 96),
    ImColor(41, 128, 185),
    ImColor(142, 68, 173),
    ImColor(44, 62, 80),
    ImColor(243, 156, 18),
    ImColor(211, 84, 0),
    ImColor(192, 57, 43),
};


static ImColor colorFromName(std::string name) {
  // Calculate the sum of ASCII values of characters in the name
  int ascii_sum = 0;
  for (char c : name) {
    ascii_sum += c;
  }
  
  // Use the sum to index into the colors vector, using modulo to wrap around
  int index = ascii_sum % nodeColors.size();
  return nodeColors[index];
}

static ImColor colorFromInt(int i) {
  int index = i % nodeColors.size();
  return nodeColors[index];
}

static ImColor colorScaledBy(ImColor color, float scale) {
  return ImColor(color.Value.x * scale, color.Value.y * scale, color.Value.z * scale, color.Value.w * scale);
}


#endif /* Colors_h */
