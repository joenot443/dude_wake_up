//
//  ShaderChainerView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/18/22.
//

#ifndef ShaderChainerView_hpp
#define ShaderChainerView_hpp

#include <stdio.h>
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

  
  std::shared_ptr<ShaderChainer> shaderChainer;
  
  void setShaderChainer(std::shared_ptr<ShaderChainer> chainer);
    
  ShaderChainerView(std::shared_ptr<ShaderChainer> shaderChainer)
  : shaderChainer(shaderChainer)
  {};
  
private:
  void styleWindow();
  void pushSelectedButtonStyle();
  void drawShaderButton(std::shared_ptr<Shader> shader, bool selected);
  void drawChainer();
  void drawShaderDropZone();
  
  ShaderType selectedShaderType = ShaderTypeNone;
};


#endif /* ShaderChainerView_hpp */
