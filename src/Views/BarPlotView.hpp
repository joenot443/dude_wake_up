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

struct BarPlotView {
public:
  static void draw(std::vector<float> data, std::string id);
};


#endif /* BarPlotView_hpp */
