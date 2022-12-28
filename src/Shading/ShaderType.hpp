//
//  ShaderType.h
//  dude_wake_up
//
//  Created by Joe Crozier on 10/19/22.
//
// clang-format off

#ifndef ShaderType_h
#define ShaderType_h

#include <string>

enum ShaderType {
  ShaderTypeNone,        
  ShaderTypeHSB,         
  ShaderTypeBlur,        
  ShaderTypePixelate,    
  ShaderTypeGlitch,      
  ShaderTypeMirror,      
  ShaderTypeTransform,   
  ShaderTypeFeedback,    
  ShaderTypeAscii,       
  ShaderTypeKaleidoscope,
  ShaderTypeTile,        
  ShaderTypeMix,
  ShaderTypeRGBShift,
  ShaderTypeWobble,
  ShaderTypeSlider,
  ShaderTypeRings,
  ShaderTypeAudioWaveform,
  ShaderTypeAudioBumper,
  ShaderTypeAudioMountains,
};

static const ShaderType AvailableShaderTypes[] = {
  // Available shaders
  ShaderTypeAudioMountains, // Generated
  ShaderTypeAudioBumper, // Generated
  ShaderTypeAudioWaveform, // Generated
  ShaderTypeSlider, // Generated
  ShaderTypeWobble, // Generated
  ShaderTypeRGBShift, // Generated
  ShaderTypeHSB, 
  ShaderTypeBlur, 
  ShaderTypePixelate, 
  ShaderTypeMirror, 
  ShaderTypeTransform, 
  ShaderTypeFeedback, 
  ShaderTypeAscii, 
  ShaderTypeKaleidoscope, 
  ShaderTypeTile, 
  ShaderTypeMix
};

static std::string shaderTypeName(ShaderType type) {
  switch (type) {
    // Shader names
    case ShaderTypeAudioMountains:
      return "AudioMountains";
    case ShaderTypeAudioBumper:
      return "AudioBumper";
    case ShaderTypeAudioWaveform:
      return "AudioWaveform";
    case ShaderTypeRings:
      return "Rings";
    case ShaderTypeSlider:
      return "Slider";
    case ShaderTypeWobble:
      return "Wobble";
    case ShaderTypeRGBShift:
      return "RGBShift";
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
    case ShaderTypeTile:
      return "Tile";
    case ShaderTypeMix:
      return "Mix";
  }

  return "";
}

#endif /* ShaderType_h */
// clang-format on
