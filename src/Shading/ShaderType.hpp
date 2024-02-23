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
#include <vector>

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
  ShaderTypeCircle,
  ShaderTypeMask,
  ShaderTypeLumaMaskMaker,
  ShaderTypeDancingSquares,
  ShaderTypePlasmaTwo,
  ShaderTypeCurlySquares,
  ShaderTypeColorKeyMaskMaker,
  ShaderTypeFrequencyVisualizer,
  ShaderTypeWarp,
  ShaderTypeHilbert,
  ShaderType16bit,
  ShaderTypeSolidColor,
  ShaderTypeMinMixer,
  ShaderTypeSlidingFrame,
  ShaderTypeStaticFrame,
  ShaderTypeGameboy,
  ShaderTypeLumaFeedback,
  ShaderTypePaint,
  ShaderTypeTriple,
  ShaderTypeVertex,
  ShaderTypeBounce,
  ShaderTypeCirclePath,
  ShaderTypeVHS,
  ShaderTypeCanny,
  ShaderTypeSnowfall,
  ShaderTypeTextureMask,
  ShaderTypeLimbo,
  ShaderTypeAudioCircle,
  ShaderTypeSmokeRing,
  ShaderTypeDoubleSwirl,
};

static const ShaderType AvailableBasicShaderTypes[] = {
  ShaderTypeHSB,
  ShaderTypeSolidColor,
  ShaderTypeBlur,
  ShaderTypePixelate, 
  ShaderTypeMirror,
  ShaderTypeGlitch,
  ShaderType16bit,
  ShaderTypePaint
};

static const ShaderType AvailableMixShaderTypes[] = {
  ShaderTypeMix,
  ShaderTypeMinMixer,
  ShaderTypeSlidingFrame,
  ShaderTypeStaticFrame,
  ShaderTypeFeedback,
  ShaderTypeLumaFeedback,
};

static const ShaderType AvailableMaskShaderTypes[] = {
  ShaderTypeMask,
  ShaderTypeTextureMask,
  ShaderTypeLumaMaskMaker,
  ShaderTypeColorKeyMaskMaker
};

static const ShaderType AvailableTransformShaderTypes[] = {
  ShaderTypeSnowfall,
  ShaderTypeTransform,
  ShaderTypeBounce,
  ShaderTypeCirclePath,
  ShaderTypeWobble,
  ShaderTypeTriple,
  ShaderTypeFishEye, // Generated
  ShaderTypeTile,
  ShaderTypeKaleidoscope,
  ShaderTypeSlider, // Generated
  ShaderTypeLiquid,
};

static const ShaderType AvailableFilterShaderTypes[] = {
  ShaderTypeCanny,
  ShaderTypeGameboy,
  ShaderTypeRainbowRotator, // Generated
  ShaderTypeRGBShift, // Generated
  ShaderTypeAscii,
  ShaderTypeDither, // Generated
  ShaderTypeSobel, // Generated
  ShaderTypeColorPass, // Generated
  ShaderTypeHalfTone, // Generated
  ShaderTypeCrosshatch, // Generated
  ShaderTypeVHS,
};

static std::vector<ShaderType> AllShaderTypes() {
  std::vector<ShaderType> combinedShaderTypes = {};

  for (const ShaderType& shader : AvailableBasicShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }

  for (const ShaderType& shader : AvailableMixShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }

  for (const ShaderType& shader : AvailableTransformShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }

  for (const ShaderType& shader : AvailableFilterShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }
  
  for (const ShaderType& shader : AvailableMaskShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }

  return combinedShaderTypes;
}

static std::string shaderTypeName(ShaderType type) {
  switch (type) {
    // Shader names
    case ShaderTypeDoubleSwirl:
      return "DoubleSwirl";
    case ShaderTypeSmokeRing:
      return "SmokeRing";
    case ShaderTypeAudioCircle:
      return "AudioCircle";
    case ShaderTypeLimbo:
      return "Limbo";
    case ShaderTypeTextureMask:
      return "TextureMask";
    case ShaderTypeSnowfall:
      return "Snowfall";
    case ShaderTypeCanny:
      return "Canny";
    case ShaderTypeVHS:
      return "VHS";
    case ShaderTypeCirclePath:
      return "CirclePath";
    case ShaderTypeBounce:
      return "Bounce";
    case ShaderTypeVertex:
      return "Vertex";
    case ShaderTypeTriple:
      return "Triple";
    case ShaderTypePaint:
      return "Paint";
    case ShaderTypeLumaFeedback:
      return "LumaFeedback";
    case ShaderTypeGameboy:
      return "Gameboy";
    case ShaderTypeStaticFrame:
      return "StaticFrame";
    case ShaderTypeSlidingFrame:
      return "SlidingFrame";
    case ShaderTypeMinMixer:
      return "MinMixer";
    case ShaderTypeSolidColor:
      return "SolidColor";
    case ShaderType16bit:
      return "16bit";
    case ShaderTypeHilbert:
      return "Hilbert";
    case ShaderTypeWarp:
      return "Warp";
    case ShaderTypeFrequencyVisualizer:
      return "FrequencyVisualizer";
    case ShaderTypeColorKeyMaskMaker:
      return "ColorMask";
    case ShaderTypeCurlySquares:
      return "CurlySquares";
    case ShaderTypePlasmaTwo:
      return "PlasmaTwo";
    case ShaderTypeDancingSquares:
      return "DancingSquares";
    case ShaderTypeLumaMaskMaker:
      return "LumaMask";
    case ShaderTypeMask:
      return "Mask";
    case ShaderTypeCircle:
      return "Circle";
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
    case ShaderTypePlasma:
      return "Plasma";
    case ShaderTypeFuji:
      return "Fuji";
    case ShaderTypeFractal:
      return "Fractal";
    case ShaderTypeClouds:
      return "Clouds";
    case ShaderTypeMelter:
      return "Melter";
  }

  return "";
}

static bool shaderTypeSupportsAux(ShaderType type) {
  if (type == ShaderTypeMix ||
      type == ShaderTypeFeedback ||
      type == ShaderTypeLumaFeedback ||
      type == ShaderTypeSlidingFrame ||
      type == ShaderTypeStaticFrame ||
      type == ShaderTypeMinMixer)
  {return true;}
  return false;
}

static bool shaderTypeSupportsMask(ShaderType type) {
  if (type == ShaderTypeMask)
  {return true;}
  return false;
}

static bool shaderTypeSupportsFeedback(ShaderType type) {
  if (type == ShaderTypeFeedback || type == ShaderTypeLumaFeedback)
  {return true;}
  return false;
}

#endif /* ShaderType_h */
// clang-format on
