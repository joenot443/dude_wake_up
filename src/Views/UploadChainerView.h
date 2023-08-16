//
//  UploadChainerView.h
//  dude_wake_up
//
//  Created by Joe Crozier on 3/20/23.
//

#ifndef UploadChainerView_h
#define UploadChainerView_h

#include <stdio.h>
#include "ShaderChainer.hpp"

struct UploadChainerView {
  static void draw(std::shared_ptr<ShaderChainer> shaderChainer);
};

#endif /* UploadChainerView_h */
