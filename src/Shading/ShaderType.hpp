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
  ShaderTypeSwirlingSoul,
  ShaderTypeCubify,
  ShaderTypeGyroids,
  ShaderTypeAlphaMix,
  ShaderTypeColorSwap,
  ShaderTypeGridRun,
  ShaderTypeColorStepper,
  ShaderTypeMultiMix,
  ShaderTypeRotate,
  ShaderTypeSwitcher,
  ShaderTypeOverlay,
  ShaderTypePieSplit,
  ShaderTypeFullHouse,
  ShaderTypeBlend,
  ShaderTypeReflector,
  ShaderTypeWarpspeed,
  ShaderTypeGodRay,
  ShaderTypeVoronoiColumns,
  ShaderTypeCore,
  ShaderTypeTwistedCubes,
  ShaderTypeTwistedTrip,
  ShaderTypeOldTV,
  ShaderTypeOneBitDither,
  ShaderTypeDirtyPlasma,
  ShaderTypePixelPlay,
  ShaderTypeTraceAudio,
  ShaderTypeSwirl,
  ShaderTypeIsoFract,
};

static const ShaderType AvailableBasicShaderTypes[] = {
  ShaderTypeHSB,
  ShaderTypeSolidColor,
  ShaderTypeBlur,
  ShaderTypePixelate, 
  ShaderTypeMirror,
  ShaderType16bit,
  ShaderTypePaint,
  ShaderTypeColorStepper
};

static const ShaderType AvailableMixShaderTypes[] = {
  ShaderTypeBlend,
  ShaderTypePieSplit,
  ShaderTypeOverlay,
  ShaderTypeSwitcher,
  ShaderTypeColorSwap,
  ShaderTypeMix,
  ShaderTypeMinMixer,
  ShaderTypeMultiMix,
  ShaderTypeSlidingFrame,
  ShaderTypeStaticFrame,
  ShaderTypeFeedback,
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
  ShaderTypeRotate,
  ShaderTypeBounce,
  ShaderTypeCirclePath,
  ShaderTypeWobble,
  ShaderTypeTriple,
  ShaderTypeFishEye, // Generated
  ShaderTypeTile,
  ShaderTypeKaleidoscope,
  ShaderTypeSlider, // Generated
  ShaderTypeLiquid,
  ShaderTypeSwirl,
};

static const ShaderType AvailableFilterShaderTypes[] = {
  ShaderTypeCubify,
  ShaderTypeGodRay,
  ShaderTypeCanny,
  ShaderTypeGameboy,
  ShaderTypeGlitch,
  ShaderTypeRainbowRotator, // Generated
  ShaderTypeRGBShift, // Generated
  ShaderTypeAscii,
  ShaderTypeDither, // Generated
  ShaderTypeSobel, // Generated
  ShaderTypeColorPass, // Generated
  ShaderTypeHalfTone, // Generated
  ShaderTypeCrosshatch, // Generated
  ShaderTypeVHS,
  ShaderTypeOldTV,
  ShaderTypeOneBitDither,
  ShaderTypePixelPlay,
  ShaderTypeTraceAudio,
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
  switch (type) { // ShaderNames
    case ShaderTypeIsoFract:
      return "IsoFract";
    case ShaderTypeSwirl:
      return "Swirl";
    case ShaderTypeTraceAudio:
      return "TraceAudio";
    case ShaderTypePixelPlay:
      return "PixelPlay";
    case ShaderTypeDirtyPlasma:
      return "DirtyPlasma";
    case ShaderTypeOneBitDither:
      return "OneBitDither";
    case ShaderTypeOldTV:
      return "OldTV";
    case ShaderTypeTwistedTrip:
      return "TwistedTrip";
    case ShaderTypeTwistedCubes:
      return "TwistedCubes";
    case ShaderTypeCore:
      return "Core";
    case ShaderTypeVoronoiColumns:
      return "VoronoiColumns";
    case ShaderTypeGodRay:
      return "GodRay";
    case ShaderTypeWarpspeed:
      return "Warpspeed";
    case ShaderTypeReflector:
      return "Reflector";
    case ShaderTypeBlend:
      return "Blend";
    case ShaderTypeFullHouse:
      return "FullHouse";
    case ShaderTypePieSplit:
      return "PieSplit";
    case ShaderTypeOverlay:
      return "Overlay";
    case ShaderTypeSwitcher:
      return "Switcher";
    case ShaderTypeRotate:
      return "Rotate";
    case ShaderTypeMultiMix:
      return "MultiMix";
    case ShaderTypeColorStepper:
      return "ColorStepper";
    case ShaderTypeGridRun:
      return "GridRun";
    case ShaderTypeColorSwap:
      return "ColorSwap";
    case ShaderTypeAlphaMix:
      return "AlphaMix";
    case ShaderTypeGyroids:
      return "Gyroids";
    case ShaderTypeCubify:
      return "Cubify";
    case ShaderTypeSwirlingSoul:
      return "Swirling Soul";
    case ShaderTypeDoubleSwirl:
      return "Double Swirl";
    case ShaderTypeSmokeRing:
      return "Smoke Ring";
    case ShaderTypeAudioCircle:
      return "Audio Circle";
    case ShaderTypeLimbo:
      return "Limbo";
    case ShaderTypeTextureMask:
      return "Texture Mask";
    case ShaderTypeSnowfall:
      return "Snowfall";
    case ShaderTypeCanny:
      return "Canny";
    case ShaderTypeVHS:
      return "VHS";
    case ShaderTypeCirclePath:
      return "Circle Path";
    case ShaderTypeBounce:
      return "Bounce";
    case ShaderTypeVertex:
      return "Vertex";
    case ShaderTypeTriple:
      return "Triple";
    case ShaderTypePaint:
      return "Paint";
    case ShaderTypeGameboy:
      return "Gameboy";
    case ShaderTypeStaticFrame:
      return "Static Frame";
    case ShaderTypeSlidingFrame:
      return "Sliding Frame";
    case ShaderTypeMinMixer:
      return "Min Mixer";
    case ShaderTypeSolidColor:
      return "Color Overlay";
    case ShaderType16bit:
      return "16bit";
    case ShaderTypeHilbert:
      return "Hilbert";
    case ShaderTypeWarp:
      return "Warp";
    case ShaderTypeFrequencyVisualizer:
      return "Frequency Visualizer";
    case ShaderTypeColorKeyMaskMaker:
      return "Color Mask";
    case ShaderTypeCurlySquares:
      return "Curly Squares";
    case ShaderTypePlasmaTwo:
      return "Plasma Two";
    case ShaderTypeDancingSquares:
      return "Dancing Squares";
    case ShaderTypeLumaMaskMaker:
      return "Luma Mask";
    case ShaderTypeMask:
      return "Mask";
    case ShaderTypeCircle:
      return "Circle";
    case ShaderTypeCrosshatch:
      return "Cross hatch";
    case ShaderTypeHalfTone:
      return "Half Tone";
    case ShaderTypeTissue:
      return "Tissue";
    case ShaderTypePsycurves:
      return "Psycurves";
    case ShaderTypeFishEye:
      return "Fish Eye";
    case ShaderTypeColorPass:
      return "Color Pass";
    case ShaderTypeSobel:
      return "Sobel";
    case ShaderTypeTriangleMap:
      return "Triangle Map";
    case ShaderTypeLiquid:
      return "Liquid";
    case ShaderTypeDisco:
      return "Disco";
    case ShaderTypeOctahedron:
      return "Octahedron";
    case ShaderTypeVanGogh:
      return "Van Gogh";
    case ShaderTypeRubiks:
      return "Rubiks";
    case ShaderTypeRainbowRotator:
      return "Rainbow Rotator";
    case ShaderTypeDither:
      return "Dither";
    case ShaderTypeMountains:
      return "Mountains";
    case ShaderTypeGalaxy:
      return "Galaxy";
    case ShaderTypeAudioMountains:
      return "Audio Mountains";
    case ShaderTypeAudioBumper:
      return "Audio Bumper";
    case ShaderTypeAudioWaveform:
      return "Audio Waveform";
    case ShaderTypeRings:
      return "Rings";
    case ShaderTypeSlider:
      return "Slider";
    case ShaderTypeWobble:
      return "Wobble";
    case ShaderTypeRGBShift:
      return "RGB Shift";
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
    default:
      return "Unknown Shader"; // Default case if none of the cases match
  }
}

static bool shaderTypeSupportsAux(ShaderType type) {
  if (type == ShaderTypeMix ||
      type == ShaderTypeFeedback ||
      type == ShaderTypeSlidingFrame ||
      type == ShaderTypeStaticFrame ||
      type == ShaderTypeColorSwap || 
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
  if (type == ShaderTypeFeedback)
  {return true;}
  return false;
}

#endif /* ShaderType_h */
// clang-format on
