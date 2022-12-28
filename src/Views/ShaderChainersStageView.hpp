//
//  ShaderChainersStageView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/22/22.
//

#ifndef ShaderChainersStageView_hpp
#define ShaderChainersStageView_hpp

#include "ShaderChainer.hpp"
#include "ShaderChainerView.hpp"
#include <stdio.h>

class ShaderChainersStageView {
public:
  void setup();
  void update();
  void draw();
  
private:
  void generateChainerViews();
  
  std::vector<std::shared_ptr<ShaderChainer>> chainers;
  std::vector<ShaderChainerView> chainerViews;
};

#endif /* ShaderChainersStageView_hpp */
