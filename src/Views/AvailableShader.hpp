//
//  AvailableShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#ifndef AvailableShader_hpp
#define AvailableShader_hpp

#include <stdio.h>
#include "ShaderType.hpp"
#include "ofMain.h"

struct AvailableShader {
  ShaderType type;
  std::string name;
  std::shared_ptr<ofTexture> preview;
  
  std::shared_ptr<ofFbo> basePreview;
  void generatePreview();
  void setBasePreview(std::shared_ptr<ofFbo> basePreview);

  AvailableShader(ShaderType type, std::string name);
};

#endif /* AvailableShader_hpp */
