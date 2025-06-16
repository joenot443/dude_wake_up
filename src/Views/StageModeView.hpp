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
#include "VideoSource.hpp"

class StageModeView {
private:
  std::vector<std::shared_ptr<FavoriteParameter>> favoriteParams;
  std::vector<std::shared_ptr<Parameter>> allParams;
  
public:
  void setup();
  void update();
  void draw();
  void drawHelp();
  
  void drawActionBar();
  void drawAllParams(int itemsPerRow);
  void drawFavoriteParams(int itemsPerRow);
  void drawOutputShader(std::shared_ptr<Shader> output);
  void drawOutputVideoSource(std::shared_ptr<VideoSource> output);
  void clear();
};


#endif /* StageModeView_hpp */
