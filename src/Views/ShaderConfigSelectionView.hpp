//
//  ShaderConfigSelectionView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/4/23.
//

#ifndef ShaderConfigSelectionView_hpp
#define ShaderConfigSelectionView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Shader.hpp"

struct ShaderConfigSelectionView {
public:
  static void draw(Shader *shader);
};

#endif /* ShaderConfigSelectionView_hpp */
