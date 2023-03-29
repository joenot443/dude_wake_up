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
  ShaderTypeGalaxy,
  ShaderTypeMountains,
  ShaderTypeDither,
  ShaderTypeRainbowRotator,
  ShaderTypeRubiks,
  ShaderTypePlasma,
  ShaderTypeVanGogh,
  ShaderTypeFuji,
  ShaderTypeFractal,
  ShaderTypeClouds,
  ShaderTypeMelter,
  ShaderTypeOctahedron,
  ShaderTypeDisco,
  ShaderTypeLiquid,
  ShaderTypeTriangleMap,
  ShaderTypeSobel,
  ShaderTypeColorPass,
  ShaderTypeFishEye,
  ShaderTypePsycurves,
  ShaderTypeTissue,
  ShaderTypeHalfTone,
  ShaderTypeCrosshatch,
};

static const ShaderType AvailableBasicShaderTypes[] = {
  ShaderTypeHSB,
  ShaderTypeBlur, 
  ShaderTypePixelate, 
  ShaderTypeMirror,
  ShaderTypeGlitch,
};

static const ShaderType AvailableMixShaderTypes[] = {
  ShaderTypeMix,
  ShaderTypeFeedback,
};

static const ShaderType AvailableTransformShaderTypes[] = {
  ShaderTypeTransform,
  ShaderTypeWobble,
  ShaderTypeFishEye, // Generated
  ShaderTypeTile,
  ShaderTypeKaleidoscope,
  ShaderTypeSlider, // Generated
  ShaderTypeLiquid,
};

static const ShaderType AvailableFilterShaderTypes[] = {
  ShaderTypeRainbowRotator, // Generated
  ShaderTypeRGBShift, // Generated
  ShaderTypeAscii,
  ShaderTypeDither, // Generated
  ShaderTypeSobel, // Generated
  ShaderTypeColorPass, // Generated
  ShaderTypeHalfTone, // Generated
  ShaderTypeCrosshatch, // Generated
};

static std::string shaderTypeName(ShaderType type) {
  switch (type) {
    // Shader names
    case ShaderTypeCrosshatch:
      return "Crosshatch";
    case ShaderTypeHalfTone:
      return "HalfTone";
    case ShaderTypeTissue:
      return "Tissue";
    case ShaderTypePsycurves:
      return "Psycurves";
    case ShaderTypeFishEye:
      return "FishEye";
    case ShaderTypeColorPass:
      return "ColorPass";
    case ShaderTypeSobel:
      return "Sobel";
    case ShaderTypeTriangleMap:
      return "TriangleMap";
    case ShaderTypeLiquid:
      return "Liquid";
    case ShaderTypeDisco:
      return "Disco";
    case ShaderTypeOctahedron:
      return "Octahedron";
    case ShaderTypeVanGogh:
      return "VanGogh";
    case ShaderTypeRubiks:
      return "Rubiks";
    case ShaderTypeRainbowRotator:
      return "RainbowRotator";
    case ShaderTypeDither:
      return "Dither";
    case ShaderTypeMountains:
      return "Mountains";
    case ShaderTypeGalaxy:
      return "Galaxy";
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

static bool shaderTypeSupportsAux(ShaderType type) {
  if (type == ShaderTypeMix ||
    type == ShaderTypeFeedback)
  {return true;}
  return false;
}

#endif /* ShaderType_h */
// clang-format on
