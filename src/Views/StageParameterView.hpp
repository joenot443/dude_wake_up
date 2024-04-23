//
//  StageParameterView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#ifndef StageParameterView_hpp
#define StageParameterView_hpp

#include <stdio.h>
#include "Parameter.hpp"
#include "imgui.h"
#include "FavoriteParameter.hpp"

static const ImVec2 StageParameterViewSize = ImVec2(200,200);

class StageParameterView {
private:
public:
  static void draw(std::shared_ptr<FavoriteParameter> favoriteParameter);
};


#endif /* StageParameterView_hpp */
