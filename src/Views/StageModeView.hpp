//
//  StageModeView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#ifndef StageModeView_hpp
#define StageModeView_hpp

#include <stdio.h>
#include "FavoriteParameter.hpp"
#include "Shader.hpp"

class StageModeView {
private:
  std::vector<std::shared_ptr<FavoriteParameter>> favoriteParams;
  
public:
  void setup();
  void update();
  void draw();
  
  void drawActionButtons();
  void drawOutput(std::shared_ptr<Shader> output);
};


#endif /* StageModeView_hpp */
