//
//  ShaderType.h
//  dude_wake_up
//
//  Created by Joe Crozier on 10/19/22.
//

#ifndef ShaderType_h
#define ShaderType_h

#include <string>

enum ShaderType {
  ShaderTypeNone,
  ShaderTypeHSB,
  ShaderTypeBlur,
  ShaderTypePixelate,
  ShaderTypeGlitch,
  ShaderTypeFeedback,
};

static std::string shaderTypeName(ShaderType type) {
  switch (type) {
    case ShaderTypeNone:
      return "Select Shader";
    case ShaderTypeHSB:
      return "HSB";
    case ShaderTypeBlur:
      return "Blur";
    case ShaderTypePixelate:
      return "Pixelate";
    case ShaderTypeGlitch:
      return "Glitch";
    case ShaderTypeFeedback:
      return "Feedback";
  }
}

#endif /* ShaderType_h */
