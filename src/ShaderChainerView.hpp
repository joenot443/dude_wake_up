//
//  ShaderChainerView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/18/22.
//

#ifndef ShaderChainerView_hpp
#define ShaderChainerView_hpp

#include <stdio.h>
#include "VideoSettings.h"
#include "Shader.hpp"
#include "FeedbackSettingsView.hpp"
#include "AudioSettings.h"
#include "ShaderType.h"
#include "FeedbackShader.hpp"

struct ShaderChainerView {
public:
  void setup();
  void update();
  void draw();
  void teardown();

  
  VideoSettings *videoSettings;
  std::vector<Shader *> *shaders;
  std::vector<FeedbackShader *> *feedbackShaders;
  Shader *selectedShader;
    
  ShaderChainerView(VideoSettings *videoSettings,
                    std::vector<Shader *> *shaders,
                    std::vector<FeedbackShader *> *feedbackShaders)
  : videoSettings(videoSettings),
  shaders(shaders),
  feedbackShaders(feedbackShaders)
  {
    selectedShader = (*shaders)[0];
  };
  
private:
  void styleWindow();
  void pushSelectedButtonStyle();
  void drawShaderButton(Shader *shader, bool selected);
  void drawChainer();
  void addNewShader(ShaderType shaderType);
  
  ShaderType selectedShaderType = ShaderTypeNone;
};


#endif /* ShaderChainerView_hpp */
