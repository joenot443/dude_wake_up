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
  ShaderTypeCore,
  ShaderTypeTwistedCubes,
  ShaderTypeTwistedTrip,
  ShaderTypeOldTV,
  ShaderTypeOneBitDither,
  ShaderTypeDirtyPlasma,
  ShaderTypePixelPlay,
  ShaderTypeSwirl,
  ShaderTypeIsoFract,
  ShaderTypeOnOff,
  ShaderTypeMotionBlurTexture,
  ShaderTypeGlitchDigital,
  ShaderTypeGlitchRGB,
  ShaderTypeWelcomeRings,
  ShaderTypeMist,
  ShaderTypeFibers,
  ShaderTypeChromeGrill,
  ShaderTypeGlitchAudio,
  ShaderTypeDoubleBlur,
  ShaderTypeOutline,
  ShaderTypeCosmos,
  ShaderTypeFloatingSparks,
  ShaderTypeAudioBlocks,
  ShaderTypeCloudyShapes,
  ShaderTypeSpaceRings,
  ShaderTypeUnknownPleasures,
  ShaderTypeStarryPlanes,
  ShaderTypeFractalAudio,
  ShaderTypeFlickerAudio,
  ShaderTypeDiscoAudio,
  ShaderTypeColorWheel,
  ShaderTypePerplexion,
  ShaderTypeOctagrams,
  ShaderTypeHeptagons,
  ShaderTypeBreathe,
  ShaderTypeAerogel,
  ShaderTypeScratchy,
  ShaderTypeDiffractor,
  ShaderTypeAutotangent,
  ShaderTypePlasmor,
  ShaderTypeColoredDrops,
  ShaderTypeSimpleShape,
  ShaderTypeBackground,
  ShaderTypeComicbook,
  ShaderTypeSimpleBars,
  ShaderTypeSpiral,
  ShaderTypeWeb,
  ShaderTypeStellar,
  ShaderTypeSpiralWhirlpool,
  ShaderTypeStrangeScreen,
  ShaderTypeCompanions,
  ShaderTypeWavy,
  ShaderTypePixelAudioParty,
};

static const ShaderType AvailableBasicShaderTypes[] = {
  ShaderTypeHSB,
  ShaderTypeSolidColor,
  ShaderTypeDoubleBlur,
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

static const ShaderType AvailableGlitchShaderTypes[] = {
  ShaderTypeGlitch,
  ShaderTypeGlitchRGB,
  ShaderTypePixelPlay,
  ShaderTypeGlitchDigital,
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
  ShaderTypeWavy,
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
      return "SimpleBars";
    case ShaderTypeComicbook:
      return "Comicbook";
    case ShaderTypeBackground:
      return "Background";
    case ShaderTypeSimpleShape:
      return "SimpleShape";
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
      return "Disco";
    case ShaderTypeFlickerAudio:
      return "Flicker";
    case ShaderTypeFractalAudio:
      return "Fractal";
    case ShaderTypeStarryPlanes:
      return "Stars";
    case ShaderTypeUnknownPleasures:
      return "Britpop";
    case ShaderTypeSpaceRings:
      return "Space Rings";
    case ShaderTypeCloudyShapes:
      return "Cloudy Shapes";
    case ShaderTypeAudioBlocks:
      return "Blocks";
    case ShaderTypeFloatingSparks:
      return "Sparks";
    case ShaderTypeCosmos:
      return "Cosmos";
    case ShaderTypeOutline:
      return "Outline";
    case ShaderTypeDoubleBlur:
      return "Blur";
    case ShaderTypeGlitchAudio:
      return "Glitchy";
    case ShaderTypeChromeGrill:
      return "Chrome Grill";
    case ShaderTypeFibers:
      return "Fibers";
    case ShaderTypeMist:
      return "Mist";
    case ShaderTypeWelcomeRings:
      return "WelcomeRings";
    case ShaderTypeGlitchRGB:
      return "RGB";
    case ShaderTypeGlitchDigital:
      return "Digital";
    case ShaderTypeMotionBlurTexture:
      return "Motion Blur";
    case ShaderTypeOnOff:
      return "OnOff";
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
      return "Min-Mixer";
    case ShaderTypeSolidColor:
      return "Color Overlay";
    case ShaderType16bit:
      return "16bit";
    case ShaderTypeHilbert:
      return "Hilbert";
    case ShaderTypeWarp:
      return "Warp";
    case ShaderTypeFrequencyVisualizer:
      return "Visualizer";
    case ShaderTypeColorKeyMaskMaker:
      return "ColorMask";
    case ShaderTypeCurlySquares:
      return "Curly";
    case ShaderTypePlasmaTwo:
      return "Plasma Two";
    case ShaderTypeDancingSquares:
      return "Dancing Squares";
    case ShaderTypeLumaMaskMaker:
      return "LumaMask";
    case ShaderTypeMask:
      return "Mask";
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
    case ShaderTypeVanGogh:
      return "VanGogh";
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

// clang-format on
#endif
