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


static ImColor SourceNodeColor = ImColor(174, 32, 18);
static ImColor ShaderNodeColor = ImColor(0, 95, 115);
static ImColor AuxNodeColor = ImColor(202, 103, 2);
static ImColor MaskNodeColor = ImColor(54, 78, 2);
static ImColor WarningColor = ImColor(233, 196, 106);
static ImColor GoodColor = ImColor(46, 204, 113);

static ImColor colorFromName(std::string name) {
  std::vector<ImColor> colors = {
     ImColor(73, 36, 62),
    ImColor(112, 66, 100),
    ImColor(187, 132, 147),
    ImColor(219, 175, 160)
  };
  
  // Calculate the sum of ASCII values of characters in the name
  int ascii_sum = 0;
  for (char c : name) {
    ascii_sum += c;
  }
  
  // Use the sum to index into the colors vector, using modulo to wrap around
  int index = ascii_sum % colors.size();
  return colors[index];
}


#endif /* Colors_h */
