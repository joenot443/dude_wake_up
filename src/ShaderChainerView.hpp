//
//  ShaderChainerView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/18/22.
//

#ifndef ShaderChainerView_hpp
#define ShaderChainerView_hpp

#include <stdio.h>
#include "VideoSettings.hpp"
#include "Shader.hpp"
#include "AudioSettings.hpp"
#include "ShaderType.hpp"
#include "ShaderChainer.hpp"
#include "FeedbackShader.hpp"

struct ShaderChainerView {
public:
  void setup();
  void update();
  void draw();
  void teardown();

  
  VideoSettings *videoSettings;
  ShaderChainer *shaderChainer;
  Shader *selectedShader = NULL;
  
  // From ShaderChainer
  std::vector<Shader *> *shaders;
  
  void setShaderChainer(ShaderChainer *chainer);
    
  ShaderChainerView(VideoSettings *videoSettings,
                    ShaderChainer *shaderChainer)
  : videoSettings(videoSettings),
  shaders(&shaderChainer->shaders),
  shaderChainer(shaderChainer)
  {};
  
private:
  void styleWindow();
  void pushSelectedButtonStyle();
  void drawShaderButton(Shader *shader, bool selected);
  void drawChainer();
  
  ShaderType selectedShaderType = ShaderTypeNone;
};


#endif /* ShaderChainerView_hpp */
