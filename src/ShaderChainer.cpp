//
//  ShaderChainer.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#include "ShaderChainer.hpp"
#include "ofMain.h"
#include "Console.hpp"

ofFbo ShaderChainer::fboChainingShaders(std::vector<Shader*> *shaders, ofFbo texture) {
  auto ping = ofFbo();
  if (!ping.isAllocated()) {
    ping.allocate(texture.getWidth(), texture.getHeight());
  } else {
    ping.begin();
    ofClear(255, 255, 255);
    ping.end();
  }
  
  ofFbo pong = texture;
  
  ofFbo *canv = &ping;
  ofFbo *tex = &pong;
  
  bool flip = false;
  for (auto shader : *shaders) {
    // Skip disabled shaders
    if (!shader->enabled()) {
      continue;
    }
    
    if (flip) {
      canv = &pong;
      tex = &ping;
    } else {
      canv = &ping;
      tex = &pong;
    }
    shader->shade(tex, canv);
    tex->begin();
    ofClear(255, 255, 255);
    tex->end();
    flip = !flip;
  }
  return *canv;
}
