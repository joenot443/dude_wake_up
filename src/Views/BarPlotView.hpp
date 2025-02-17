//
//  BarPlotView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/27/22.
//

#ifndef BarPlotView_hpp
#define BarPlotView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "implot.h"

struct BarPlotView {
public:
  static void draw(std::vector<float> yData, std::string id, ImVec2 size = ImVec2(200, 200));
};


#endif /* BarPlotView_hpp */
