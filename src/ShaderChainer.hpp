//
//  ShaderChainer.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ShaderChainer_hpp
#define ShaderChainer_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxImGui.h"
#include "Shader.hpp"

struct ShaderChainer {
  static ofFbo fboChainingShaders(std::vector<Shader *> *shaders, ofFbo texture);
};


#endif /* ShaderChainer_hpp */
