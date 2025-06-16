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
  ShaderTypeNone, // 0
  ShaderTypeHSB, // 1
  ShaderTypeBlur, // 2
  ShaderTypePixelate, // 3
  ShaderTypeGlitch, // 4
  ShaderTypeMirror, // 5
  ShaderTypeTransform, // 6
  ShaderTypeFeedback, // 7
  ShaderTypeAscii, // 8
  ShaderTypeKaleidoscope, // 9
  ShaderTypeTile, // 10
  ShaderTypeMix, // 11
  ShaderTypeRGBShift, // 12
  ShaderTypeWobble, // 13
  ShaderTypeSlider, // 14
  ShaderTypeRings, // 15
  ShaderTypeAudioWaveform, // 16
  ShaderTypeAudioBumper, // 17
  ShaderTypeAudioMountains, // 18
  ShaderTypeGalaxy, // 19
  ShaderTypeMountains, // 20
  ShaderTypeDither, // 21
  ShaderTypeRainbowRotator, // 22
  ShaderTypeRubiks, // 23
  ShaderTypePlasma, // 24
  ShaderTypeFuji, // 25
  ShaderTypeFractal, // 26
  ShaderTypeClouds, // 27
  ShaderTypeMelter, // 28
  ShaderTypeOctahedron, // 29
  ShaderTypeDisco, // 30
  ShaderTypeLiquid, // 31
  ShaderTypeTriangleMap, // 32
  ShaderTypeSobel, // 33
  ShaderTypeColorPass, // 34
  ShaderTypeFishEye, // 35
  ShaderTypePsycurves, // 36
  ShaderTypeTissue, // 37
  ShaderTypeHalfTone, // 38
  ShaderTypeCrosshatch, // 39
  ShaderTypeMask, // 40
  ShaderTypeLumaMaskMaker, // 41
  ShaderTypeDancingSquares, // 42
  ShaderTypePlasmaTwo, // 43
  ShaderTypeCurlySquares, // 44
  ShaderTypeColorKeyMaskMaker, // 45
  ShaderTypeFrequencyVisualizer, // 46
  ShaderTypeWarp, // 47
  ShaderTypeHilbert, // 48
  ShaderType16bit, // 49
  ShaderTypeSolidColor, // 50
  ShaderTypeMinMixer, // 51
  ShaderTypeSlidingFrame, // 52
  ShaderTypeStaticFrame, // 53
  ShaderTypeGameboy, // 54
  ShaderTypePaint, // 55
  ShaderTypeTriple, // 56
  ShaderTypeVertex, // 57
  ShaderTypeBounce, // 58
  ShaderTypeCirclePath, // 59
  ShaderTypeVHS, // 60
  ShaderTypeCanny, // 61
  ShaderTypeSnowfall, // 62
  ShaderTypeTextureMask, // 63
  ShaderTypeLimbo, // 64
  ShaderTypeAudioCircle, // 65
  ShaderTypeSmokeRing, // 66
  ShaderTypeSwirlingSoul, // 67
  ShaderTypeCubify, // 68
  ShaderTypeGyroids, // 69
  ShaderTypeAlphaMix, // 70
  ShaderTypeColorSwap, // 71
  ShaderTypeGridRun, // 72
  ShaderTypeColorStepper, // 73
  ShaderTypeMultiMix, // 74
  ShaderTypeRotate, // 75
  ShaderTypeSwitcher, // 76
  ShaderTypeOverlay, // 77
  ShaderTypePieSplit, // 78
  ShaderTypeFullHouse, // 79
  ShaderTypeBlend, // 80
  ShaderTypeReflector, // 81
  ShaderTypeWarpspeed, // 82
  ShaderTypeCore, // 83
  ShaderTypeTwistedCubes, // 84
  ShaderTypeTwistedTrip, // 85
  ShaderTypeOldTV, // 86
  ShaderTypeOneBitDither, // 87
  ShaderTypeDirtyPlasma, // 88
  ShaderTypePixelPlay, // 89
  ShaderTypeSwirl, // 90
  ShaderTypeIsoFract, // 91
  ShaderTypeOnOff, // 92
  ShaderTypeMotionBlurTexture, // 93
  ShaderTypeGlitchDigital, // 94
  ShaderTypeGlitchRGB, // 95
  ShaderTypeWelcomeRings, // 96
  ShaderTypeMist, // 97
  ShaderTypeFibers, // 98
  ShaderTypeChromeGrill, // 99
  ShaderTypeGlitchAudio, // 100
  ShaderTypeDoubleBlur, // 101
  ShaderTypeOutline, // 102
  ShaderTypeCosmos, // 103
  ShaderTypeFloatingSparks, // 104
  ShaderTypeAudioBlocks, // 105
  ShaderTypeCloudyShapes, // 106
  ShaderTypeSpaceRings, // 107
  ShaderTypeUnknownPleasures, // 108
  ShaderTypeFractalAudio, // 109
  ShaderTypeFlickerAudio, // 110
  ShaderTypeDiscoAudio, // 111
  ShaderTypeColorWheel, // 112
  ShaderTypePerplexion, // 113
  ShaderTypeOctagrams, // 114
  ShaderTypeHeptagons, // 115
  ShaderTypeBreathe, // 116
  ShaderTypeAerogel, // 117
  ShaderTypeScratchy, // 118
  ShaderTypeDiffractor, // 119
  ShaderTypeAutotangent, // 120
  ShaderTypePlasmor, // 121
  ShaderTypeColoredDrops, // 122
  ShaderTypeSimpleShape, // 123
  ShaderTypeBackground, // 124
  ShaderTypeComicbook, // 125
  ShaderTypeSimpleBars, // 126
  ShaderTypeSpiral, // 127
  ShaderTypeWeb, // 128
  ShaderTypeStellar, // 129
  ShaderTypeSpiralWhirlpool, // 130
  ShaderTypeStrangeScreen, // 131
  ShaderTypeCompanions, // 132
  ShaderTypeWavy, // 133
  ShaderTypePixelAudioParty, // 134
  ShaderTypeCloudyTunnel, // 135
  ShaderTypeFireflies, // 136
  ShaderTypeGodRay, // 137
};

static const ShaderType AvailableBasicShaderTypes[] = {
  ShaderTypeHSB,
  ShaderTypeSolidColor,
  ShaderTypeDoubleBlur,
  ShaderTypeBlur,
  ShaderTypePixelate,
  ShaderTypeMirror,
  ShaderType16bit,
  ShaderTypePaint,
  ShaderTypeColorStepper,
  ShaderTypeOnOff,
  ShaderTypeOutline
};

static const ShaderType AvailableMixShaderTypes[] = {
  ShaderTypeBlend,
  ShaderTypePieSplit,
  ShaderTypeOverlay,
  ShaderTypeSwitcher,
  ShaderTypeColorSwap,
  ShaderTypeMix,
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

static const ShaderType AvailableGlitchShaderTypes[] = {
  ShaderTypeGlitch,
  ShaderTypeGlitchRGB,
  ShaderTypePixelPlay,
  ShaderTypeGlitchDigital,
  ShaderTypeGlitchAudio,
  ShaderTypeWeb,
  ShaderTypeScratchy,
  ShaderTypeAutotangent
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
  ShaderTypeCanny,
  ShaderTypeStellar,
  ShaderTypeStrangeScreen,
  ShaderTypeComicbook,
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
};

static const ShaderType ExcludedRandomShaderTypes[] = {
  ShaderTypeTransform,
  ShaderTypeOnOff,
  ShaderTypeHSB,
  ShaderTypeGlitchAudio,
  ShaderTypeBounce,
  ShaderTypeRotate,
  ShaderTypeColorPass,
  ShaderTypeColorStepper,
  ShaderTypePaint,
  ShaderTypeTriple,
  ShaderTypeHalfTone
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
  
  for (const ShaderType& shader : AvailableGlitchShaderTypes) {
      combinedShaderTypes.push_back(shader);
  }

  return combinedShaderTypes;
}

static std::string shaderTypeName(ShaderType type) {
  switch (type) { // ShaderNames
    case ShaderTypeGodRay:
      return "GodRay";
    case ShaderTypeFireflies:
      return "Fireflies";
    case ShaderTypeCloudyTunnel:
      return "CloudyTunnel";
    case ShaderTypePixelAudioParty:
      return "PixelAudioParty";
    case ShaderTypeWavy:
      return "Wavy";
    case ShaderTypeCompanions:
      return "Companions";
    case ShaderTypeStrangeScreen:
      return "StrangeScreen";
    case ShaderTypeSpiralWhirlpool:
      return "SpiralWhirlpool";
    case ShaderTypeStellar:
      return "Stellar";
    case ShaderTypeWeb:
      return "Web";
    case ShaderTypeSpiral:
      return "Spiral";
    case ShaderTypeSimpleBars:
      return "Simple Bars";
    case ShaderTypeComicbook:
      return "Comicbook";
    case ShaderTypeBackground:
      return "Background";
    case ShaderTypeSimpleShape:
      return "Simple Shape";
    case ShaderTypeColoredDrops:
      return "Colored Drops";
    case ShaderTypePlasmor:
      return "Plasmor";
    case ShaderTypeAutotangent:
      return "Autotangent";
    case ShaderTypeDiffractor:
      return "Diffractor";
    case ShaderTypeScratchy:
      return "Scratchy";
    case ShaderTypeAerogel:
      return "Aerogel";
    case ShaderTypeBreathe:
      return "Breathe";
    case ShaderTypeHeptagons:
      return "Heptagons";
    case ShaderTypeOctagrams:
      return "Octagrams";
    case ShaderTypePerplexion:
      return "Perplexion";
    case ShaderTypeColorWheel:
      return "ColorWheel";
    case ShaderTypeDiscoAudio:
      return "Disco Audio";
    case ShaderTypeFlickerAudio:
      return "Flicker Audio";
    case ShaderTypeFractalAudio:
      return "Fractal";
    case ShaderTypeUnknownPleasures:
      return "Britpop";
    case ShaderTypeSpaceRings:
      return "Space Rings";
    case ShaderTypeCloudyShapes:
      return "Cloudy Shapes";
    case ShaderTypeAudioBlocks:
      return "Blocks";
    case ShaderTypeFloatingSparks:
      return "Floating Sparks";
    case ShaderTypeCosmos:
      return "Cosmos";
    case ShaderTypeOutline:
      return "Outline";
    case ShaderTypeDoubleBlur:
      return "Blur";
    case ShaderTypeGlitchAudio:
      return "Glitch Audio";
    case ShaderTypeChromeGrill:
      return "Chrome Grill";
    case ShaderTypeFibers:
      return "Fibers";
    case ShaderTypeMist:
      return "Mist";
    case ShaderTypeWelcomeRings:
      return "Glowing Rings";
    case ShaderTypeGlitchRGB:
      return "Glitch RGB";
    case ShaderTypeGlitchDigital:
      return "Digital";
    case ShaderTypeMotionBlurTexture:
      return "Motion Blur";
    case ShaderTypeOnOff:
      return "On Off";
    case ShaderTypeIsoFract:
      return "Isofractal";
    case ShaderTypeSwirl:
      return "Swirl";
    case ShaderTypePixelPlay:
      return "PixelPlay";
    case ShaderTypeDirtyPlasma:
      return "DirtyPlasma";
    case ShaderTypeOneBitDither:
      return "1b-Dither";
    case ShaderTypeOldTV:
      return "OldTV";
    case ShaderTypeTwistedTrip:
      return "Twister";
    case ShaderTypeTwistedCubes:
      return "Vortex Cubes";
    case ShaderTypeCore:
      return "Core";
    case ShaderTypeWarpspeed:
      return "Warpspeed";
    case ShaderTypeReflector:
      return "Reflector";
    case ShaderTypeBlend:
      return "Blend";
    case ShaderTypeFullHouse:
      return "Full House";
    case ShaderTypePieSplit:
      return "Pie Split";
    case ShaderTypeOverlay:
      return "Overlay";
    case ShaderTypeSwitcher:
      return "Switcher";
    case ShaderTypeRotate:
      return "Rotate";
    case ShaderTypeMultiMix:
      return "Multi-Mix";
    case ShaderTypeColorStepper:
      return "Color Steps";
    case ShaderTypeGridRun:
      return "Cyber Run";
    case ShaderTypeColorSwap:
      return "ColorSwap";
    case ShaderTypeGyroids:
      return "Gyroids";
    case ShaderTypeCubify:
      return "Cubify";
    case ShaderTypeSwirlingSoul:
      return "Swirling Soul";
    case ShaderTypeSmokeRing:
      return "Smoke Ring";
    case ShaderTypeLimbo:
      return "Limbo";
    case ShaderTypeTextureMask:
      return "Texture Masker";
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
      return "Curly";
    case ShaderTypePlasmaTwo:
      return "Plasma Two";
    case ShaderTypeDancingSquares:
      return "Dancing Squares";
    case ShaderTypeLumaMaskMaker:
      return "Luma Mask";
    case ShaderTypeMask:
      return "Masker";
    case ShaderTypeCrosshatch:
      return "Crosshatch";
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
      return "Triangle Land";
    case ShaderTypeLiquid:
      return "Liquid";
    case ShaderTypeDisco:
      return "Disco";
    case ShaderTypeOctahedron:
      return "Octahedron";
    case ShaderTypeRubiks:
      return "Rubiks";
    case ShaderTypeRainbowRotator:
      return "Color Spin";
    case ShaderTypeDither:
      return "Dither";
    case ShaderTypeMountains:
      return "Mountains";
    case ShaderTypeGalaxy:
      return "Galaxy";
    case ShaderTypeAudioMountains:
      return "Mountains";
    case ShaderTypeAudioBumper:
      return "Bumper";
    case ShaderTypeAudioWaveform:
      return "Waveform";
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
      return "Retro Blur";
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
      type == ShaderTypeColorSwap)
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

// clang-format on
#endif
