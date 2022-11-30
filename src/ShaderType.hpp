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
  ShaderTypeNone,         // 0
  ShaderTypeHSB,          // 1
  ShaderTypeBlur,         // 2
  ShaderTypePixelate,     // 3
  ShaderTypeGlitch,       // 4
  ShaderTypeMirror,       // 5
  ShaderTypeTransform,    // 6
  ShaderTypeFeedback,     // 7
  ShaderTypeAscii,        // 8
  ShaderTypeKaleidoscope, // 9
};

static const ShaderType AvailableShaderTypes[] = {ShaderTypeHSB, ShaderTypeBlur, ShaderTypePixelate, ShaderTypeMirror, ShaderTypeTransform, ShaderTypeFeedback, ShaderTypeAscii, ShaderTypeKaleidoscope};

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
    case ShaderTypeTransform:
      return "Transform";
    case ShaderTypeMirror:
      return "Mirror";
    case ShaderTypeFeedback:
      return "Feedback";
    case ShaderTypeAscii:
      return "ASCII";
    case ShaderTypeKaleidoscope:
      return "Kaleidoscope";
  }
}

#endif /* ShaderType_h */
