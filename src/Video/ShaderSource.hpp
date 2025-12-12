// clang-format off

#ifndef ShaderSource_hpp
#define ShaderSource_hpp

#include "AudioBumperShader.hpp"
#include "AudioGraphShader.hpp"
#include "AudioOscillatorShader.hpp"
#include "LavaShader.hpp"
#include "AudioGlowBarsShader.hpp"
#include "AudioBallShader.hpp"
#include "BubblesShader.hpp"
#include "ElectricEelsShader.hpp"
#include "SynthflyShader.hpp"
#include "SynthwaveShader.hpp"
#include "GlassyShader.hpp"
#include "SynthwaveShader.hpp"
#include "GlowBeatShader.hpp"
#include "FlamingShader.hpp"
#include "AccretionShader.hpp"
#include "FirefliesShader.hpp"
#include "CloudyTunnelShader.hpp"
#include "PixelAudioPartyShader.hpp"
#include "CompanionsShader.hpp"
#include "SpiralShader.hpp"
#include "SimpleBarsShader.hpp"
#include "SimpleShapeShader.hpp"
#include "ColoredDropsShader.hpp"
#include "PlasmorShader.hpp"
#include "DiffractorShader.hpp"
#include "AerogelShader.hpp"
#include "BreatheShader.hpp"
#include "HeptagonsShader.hpp"
#include "OctagramsShader.hpp"
#include "PerplexionShader.hpp"
#include "ColorWheelShader.hpp"
#include "DiscoAudioShader.hpp"
#include "FlickerAudioShader.hpp"
#include "FractalAudioShader.hpp"
#include "UnknownPleasuresShader.hpp"
#include "SpaceRingsShader.hpp"
#include "CloudyShapesShader.hpp"
#include "AudioBlocksShader.hpp"
#include "FloatingSparksShader.hpp"
#include "CosmosShader.hpp"
#include "GlitchAudioShader.hpp"
#include "ChromeGrillShader.hpp"
#include "FibersShader.hpp"
#include "MistShader.hpp"
#include "WelcomeRingsShader.hpp"
#include "MotionBlurTextureShader.hpp"
#include "IsoFractShader.hpp"
#include "PixelPlayShader.hpp"
#include "DirtyPlasmaShader.hpp"
#include "TwistedTripShader.hpp"
#include "TwistedCubesShader.hpp"
#include "LayoutStateService.hpp"
#include "CoreShader.hpp"
#include "WarpspeedShader.hpp"
#include "ReflectorShader.hpp"
#include "FullHouseShader.hpp"
#include "GridRunShader.hpp"
#include "GyroidsShader.hpp"
#include "SwirlingSoulShader.hpp"
#include "SmokeRingShader.hpp"
#include "LimboShader.hpp"
#include "VertexShader.hpp"
#include "SolidColorShader.hpp"
#include "HilbertShader.hpp"
#include "WarpShader.hpp"
#include "FrequencyVisualizerShader.hpp"
#include "CurlySquaresShader.hpp"
#include "PlasmaTwoShader.hpp"
#include "DancingSquaresShader.hpp"
#include "TissueShader.hpp"
#include "PsycurvesShader.hpp"
#include "TriangleMapShader.hpp"
#include "DiscoShader.hpp"
#include "OctahedronShader.hpp"
#include "RubiksShader.hpp"
#include "AudioMountainsShader.hpp"
#include "AudioWaveformShader.hpp"
#include "CloudShader.hpp"
#include "EmptyShader.hpp"
#include "FractalShader.hpp"
#include "FujiShader.hpp"
#include "GalaxyShader.hpp"
#include "MelterShader.hpp"
#include "MountainsShader.hpp"
#include "CommonViews.hpp"
#include "PlasmaShader.hpp"
#include "RingsShader.hpp"
#include "Shader.hpp"
#include "UUID.hpp"
#include "ShaderType.hpp"
#include "VideoSource.hpp"
#include <stdio.h>

using json = nlohmann::json;

enum ShaderSourceType {
  ShaderSource_FrequencyVisualizer, // 0
  ShaderSource_CurlySquares, // 1
  ShaderSource_PlasmaTwo, // 2
  ShaderSource_DancingSquares, // 3
  ShaderSource_Tissue, // 4
  ShaderSource_Psycurves, // 5
  ShaderSource_TriangleMap, // 6
  ShaderSource_Disco, // 7
  ShaderSource_Octahedron, // 8
  ShaderSource_Rubiks, // 9
  ShaderSource_Mountains, // 10
  ShaderSource_empty, // 11
  ShaderSource_plasma, // 12
  ShaderSource_fractal, // 13
  ShaderSource_fuji, // 14
  ShaderSource_clouds, // 15
  ShaderSource_melter, // 16
  ShaderSource_rings, // 17
  ShaderSource_audioWaveform, // 18
  ShaderSource_audioBumper, // 19
  ShaderSource_audioMountains, // 20
  ShaderSource_galaxy, // 21
  ShaderSource_Warp, // 22
  ShaderSource_Hilbert, // 23
  ShaderSource_SolidColor, // 24
  ShaderSource_Vertex, // 25
  ShaderSource_Limbo, // 26
  ShaderSource_AudioCircle, // 27
  ShaderSource_SmokeRing, // 28
  ShaderSource_SwirlingSoul, // 29
  ShaderSource_Gyroids, // 30
  ShaderSource_GridRun, // 31
  ShaderSource_FullHouse, // 32
  ShaderSource_Reflector, // 33
  ShaderSource_Warpspeed, // 34
  ShaderSource_Core, // 35
  ShaderSource_TwistedCubes, // 36
  ShaderSource_TwistedTrip, // 37
  ShaderSource_DirtyPlasma, // 38
  ShaderSource_IsoFract, // 39
  ShaderSource_MotionBlurTexture, // 40
  ShaderSource_WelcomeRings, // 41
  ShaderSource_Mist, // 42
  ShaderSource_Fibers, // 43
  ShaderSource_ChromeGrill, // 44
  ShaderSource_GlitchAudio, // 45
  ShaderSource_Cosmos, // 46
  ShaderSource_FloatingSparks, // 47
  ShaderSource_AudioBlocks, // 48
  ShaderSource_CloudyShapes, // 49
  ShaderSource_SpaceRings, // 50
  ShaderSource_UnknownPleasures, // 51
  ShaderSource_FractalAudio, // 52
  ShaderSource_FlickerAudio, // 53
  ShaderSource_DiscoAudio, // 54
  ShaderSource_ColorWheel, // 55
  ShaderSource_Perplexion, // 56
  ShaderSource_Octagrams, // 57
  ShaderSource_Heptagons, // 58
  ShaderSource_Breathe, // 59
  ShaderSource_Aerogel, // 60
  ShaderSource_Diffractor, // 61
  ShaderSource_Plasmor, // 62
  ShaderSource_ColoredDrops, // 63
  ShaderSource_SimpleShape, // 64
  ShaderSource_SimpleBars, // 65
  ShaderSource_Spiral, // 66
  ShaderSource_Companions, // 67
  ShaderSource_PixelAudioParty, // 68
  ShaderSource_CloudyTunnel, // 69
  ShaderSource_Fireflies, // 70
  ShaderSource_Accretion, //source enum,
  ShaderSource_Flaming, //source enum,
  ShaderSource_GlowBeat, //source enum,
  ShaderSource_Synthwave, //source enum,
  ShaderSource_Glassy, //source enum,
  ShaderSource_CubeGlow, //source enum,
  ShaderSource_Synthfly, //source enum,
  ShaderSource_ElectricEels, //source enum,
  ShaderSource_Bubbles, //source enum,
  ShaderSource_AudioBall, //source enum,
  ShaderSource_AudioGlowBars, //source enum,
  ShaderSource_Lava, //source enum,
  ShaderSource_AudioOscillator, //source enum,
  ShaderSource_AudioGraph, //source enum,
}; // End ShaderSourceType

static const ShaderSourceType AvailableShaderSourceTypes[] = {
  // Available ShaderSourceTypes
  ShaderSource_FrequencyVisualizer, // Available
  ShaderSource_CurlySquares, // Available
  ShaderSource_PlasmaTwo, // Available
  ShaderSource_DancingSquares, // Available
  ShaderSource_Tissue, // Available
  ShaderSource_Psycurves,
  ShaderSource_TriangleMap,
  ShaderSource_Disco,
  ShaderSource_Octahedron,
  ShaderSource_Rubiks,
  ShaderSource_Mountains,
  ShaderSource_plasma,
  ShaderSource_fractal,
  ShaderSource_fuji,
  ShaderSource_clouds,
  ShaderSource_melter,
  ShaderSource_rings,
  ShaderSource_audioWaveform,
  ShaderSource_audioBumper,
  ShaderSource_audioMountains,
  ShaderSource_galaxy,
  ShaderSource_Warp, // Available
  ShaderSource_SolidColor, // Available
  ShaderSource_Vertex, // Available
  ShaderSource_Limbo, // Available
  ShaderSource_SmokeRing, // Available
  ShaderSource_SwirlingSoul, // Available
  ShaderSource_Gyroids, // Available
  ShaderSource_GridRun, // Available
  ShaderSource_FullHouse, // Available
  ShaderSource_Reflector, // Available
  ShaderSource_Warpspeed, // Available
  ShaderSource_Core, // Available
  ShaderSource_CloudyShapes,
  ShaderSource_TwistedCubes, // Available
  ShaderSource_TwistedTrip, // Available
  ShaderSource_DirtyPlasma, // Available
  ShaderSource_IsoFract, // Available
  ShaderSource_MotionBlurTexture, // Available
  ShaderSource_WelcomeRings, // Available
  ShaderSource_Mist, // Available
  ShaderSource_Fibers, // Available
  ShaderSource_ChromeGrill, // Available
  ShaderSource_Cosmos, // Available
  ShaderSource_FloatingSparks, // Available
  ShaderSource_AudioBlocks, // Available
  ShaderSource_SpaceRings, // Available
  ShaderSource_UnknownPleasures, // Available
  ShaderSource_FractalAudio, // Available
  ShaderSource_FlickerAudio, // Available
  ShaderSource_DiscoAudio, // Available
  ShaderSource_ColorWheel, // Available
  ShaderSource_Perplexion, // Available
  ShaderSource_Octagrams, // Available
  ShaderSource_Heptagons, // Available
  ShaderSource_Breathe, // Available
  ShaderSource_Aerogel, // Available
  ShaderSource_Diffractor, // Available
  ShaderSource_Plasmor, // Available
  ShaderSource_ColoredDrops, // Available
  ShaderSource_SimpleShape, // Available
  ShaderSource_SimpleBars, // Available
  ShaderSource_Spiral, // Available
  ShaderSource_Companions, // Available
  ShaderSource_PixelAudioParty, // Available
//  ShaderSource_CloudyTunnel, // Available
//  ShaderSource_Fireflies, // Available
  ShaderSource_Accretion, // Available
  ShaderSource_Flaming, // Available
//  ShaderSource_GlowBeat, // Available
  ShaderSource_Synthwave, // Available
  ShaderSource_Glassy, // Available
  ShaderSource_Synthfly, // Available
  ShaderSource_ElectricEels, // Available
  ShaderSource_Bubbles, // Available
  ShaderSource_AudioBall, // Available
  ShaderSource_AudioGlowBars, // Available
  ShaderSource_Lava, // Available
  ShaderSource_AudioOscillator, // Available
  ShaderSource_AudioGraph, // Available
}; // End AvailableShaderSourceTypes

static ShaderType shaderTypeForShaderSourceType(ShaderSourceType type) {
  switch (type) {
      // shaderTypeForShaderSourceType
  case ShaderSource_AudioGraph: //type enum
    return ShaderTypeAudioGraph;
  case ShaderSource_AudioOscillator: //type enum
    return ShaderTypeAudioOscillator;
  case ShaderSource_Lava: //type enum
    return ShaderTypeLava;
  case ShaderSource_AudioGlowBars: //type enum
    return ShaderTypeAudioGlowBars;
  case ShaderSource_AudioBall: //type enum
    return ShaderTypeAudioBall;
  case ShaderSource_Bubbles: //type enum
    return ShaderTypeBubbles;
  case ShaderSource_ElectricEels: //type enum
    return ShaderTypeElectricEels;
  case ShaderSource_Synthfly: //type enum
    return ShaderTypeSynthfly;
  case ShaderSource_Glassy: //type enum
    return ShaderTypeGlassy;
  case ShaderSource_Synthwave: //type enum
    return ShaderTypeSynthwave;
  case ShaderSource_GlowBeat: //type enum
    return ShaderTypeGlowBeat;
  case ShaderSource_Flaming: //type enum
    return ShaderTypeFlaming;
  case ShaderSource_Accretion: //type enum
    return ShaderTypeAccretion;
  case ShaderSource_Fireflies: //type enum
    return ShaderTypeFireflies;
  case ShaderSource_CloudyTunnel: //type enum
    return ShaderTypeCloudyTunnel;
  case ShaderSource_PixelAudioParty: //type enum
    return ShaderTypePixelAudioParty;
  case ShaderSource_Companions: //type enum
    return ShaderTypeCompanions;
  case ShaderSource_Spiral: //type enum
    return ShaderTypeSpiral;
  case ShaderSource_SimpleBars: //type enum
    return ShaderTypeSimpleBars;
  case ShaderSource_SimpleShape: //type enum
    return ShaderTypeSimpleShape;
  case ShaderSource_ColoredDrops: //type enum
    return ShaderTypeColoredDrops;
  case ShaderSource_Plasmor: //type enum
    return ShaderTypePlasmor;
  case ShaderSource_Diffractor: //type enum
    return ShaderTypeDiffractor;
  case ShaderSource_Aerogel: //type enum
    return ShaderTypeAerogel;
  case ShaderSource_Breathe: //type enum
    return ShaderTypeBreathe;
  case ShaderSource_Heptagons: //type enum
    return ShaderTypeHeptagons;
  case ShaderSource_Octagrams: //type enum
    return ShaderTypeOctagrams;
  case ShaderSource_Perplexion: //type enum
    return ShaderTypePerplexion;
    case ShaderSource_ColorWheel: //type enum
      return ShaderTypeColorWheel;
    case ShaderSource_DiscoAudio: //type enum
      return ShaderTypeDiscoAudio;
    case ShaderSource_FlickerAudio: //type enum
      return ShaderTypeFlickerAudio;
    case ShaderSource_FractalAudio: //type enum
      return ShaderTypeFractalAudio;
    case ShaderSource_UnknownPleasures: //type enum
      return ShaderTypeUnknownPleasures;
    case ShaderSource_SpaceRings: //type enum
      return ShaderTypeSpaceRings;
    case ShaderSource_CloudyShapes: //type enum
      return ShaderTypeCloudyShapes;
    case ShaderSource_AudioBlocks: //type enum
      return ShaderTypeAudioBlocks;
    case ShaderSource_FloatingSparks: //type enum
      return ShaderTypeFloatingSparks;
    case ShaderSource_Cosmos: //type enum
      return ShaderTypeCosmos;
    case ShaderSource_GlitchAudio: //type enum
      return ShaderTypeGlitchAudio;
    case ShaderSource_ChromeGrill: //type enum
      return ShaderTypeChromeGrill;
    case ShaderSource_Fibers: //type enum
      return ShaderTypeFibers;
    case ShaderSource_Mist: //type enum
      return ShaderTypeMist;
    case ShaderSource_WelcomeRings: //type enum
      return ShaderTypeWelcomeRings;
    case ShaderSource_MotionBlurTexture: //type enum
      return ShaderTypeMotionBlurTexture;
    case ShaderSource_IsoFract: //type enum
      return ShaderTypeIsoFract;
    case ShaderSource_DirtyPlasma: //type enum
      return ShaderTypeDirtyPlasma;
    case ShaderSource_TwistedTrip: //type enum
      return ShaderTypeTwistedTrip;
    case ShaderSource_TwistedCubes: //type enum
      return ShaderTypeTwistedCubes;
    case ShaderSource_Core: //type enum
      return ShaderTypeCore;
    case ShaderSource_Warpspeed: //type enum
      return ShaderTypeWarpspeed;
    case ShaderSource_Reflector: //type enum
      return ShaderTypeReflector;
    case ShaderSource_FullHouse: //type enum
      return ShaderTypeFullHouse;
    case ShaderSource_GridRun: //type enum
      return ShaderTypeGridRun;
    case ShaderSource_Gyroids: //type enum
      return ShaderTypeGyroids;
    case ShaderSource_SwirlingSoul: //type enum
      return ShaderTypeSwirlingSoul;
    case ShaderSource_SmokeRing: //type enum
      return ShaderTypeSmokeRing;
    case ShaderSource_Limbo: //type enum
      return ShaderTypeLimbo;
    case ShaderSource_Vertex: //type enum
      return ShaderTypeVertex;
    case ShaderSource_SolidColor: //type enum
      return ShaderTypeSolidColor;
    case ShaderSource_Hilbert: //type enum
      return ShaderTypeHilbert;
    case ShaderSource_Warp: //type enum
      return ShaderTypeWarp;
    case ShaderSource_FrequencyVisualizer: //type enum
      return ShaderTypeFrequencyVisualizer;
    case ShaderSource_CurlySquares: //type enum
      return ShaderTypeCurlySquares;
    case ShaderSource_PlasmaTwo: //type enum
      return ShaderTypePlasmaTwo;
    case ShaderSource_DancingSquares: //type enum
      return ShaderTypeDancingSquares;
    case ShaderSource_Tissue: //type enum
      return ShaderTypeTissue;
    case ShaderSource_Psycurves:
      return ShaderTypePsycurves;
    case ShaderSource_TriangleMap:
      return ShaderTypeTriangleMap;
    case ShaderSource_Disco:
      return ShaderTypeDisco;
    case ShaderSource_Octahedron:
      return ShaderTypeOctahedron;
    case ShaderSource_Rubiks:
      return ShaderTypeRubiks;
    case ShaderSource_Mountains:
      return ShaderTypeMountains;
    case ShaderSource_empty:
      return ShaderTypeNone;
    case ShaderSource_plasma:
      return ShaderTypePlasma;
    case ShaderSource_fractal:
      return ShaderTypeFractal;
    case ShaderSource_fuji:
      return ShaderTypeFuji;
    case ShaderSource_clouds:
      return ShaderTypeClouds;
    case ShaderSource_melter:
      return ShaderTypeMelter;
    case ShaderSource_rings:
      return ShaderTypeRings;
    case ShaderSource_audioWaveform:
      return ShaderTypeAudioWaveform;
    case ShaderSource_audioBumper:
      return ShaderTypeAudioBumper;
    case ShaderSource_audioMountains:
      return ShaderTypeAudioMountains;
    case ShaderSource_galaxy:
      return ShaderTypeGalaxy;
    default:
      return ShaderTypeNone;
  }
}

static ShaderSourceType shaderSourceTypeForShaderType(ShaderType type) {
  switch (type) {
    // Reverse mapping from ShaderType to ShaderSourceType
    case ShaderTypeSpiral:
      return ShaderSource_Spiral;
    case ShaderTypeSimpleBars:
      return ShaderSource_SimpleBars;
    case ShaderTypeSimpleShape:
      return ShaderSource_SimpleShape;
    case ShaderTypeColoredDrops:
      return ShaderSource_ColoredDrops;
    case ShaderTypePlasmor:
      return ShaderSource_Plasmor;
    case ShaderTypeDiffractor:
      return ShaderSource_Diffractor;
    case ShaderTypeAerogel:
      return ShaderSource_Aerogel;
    case ShaderTypeBreathe:
      return ShaderSource_Breathe;
    case ShaderTypeHeptagons:
      return ShaderSource_Heptagons;
    case ShaderTypeOctagrams:
      return ShaderSource_Octagrams;
    case ShaderTypePerplexion:
      return ShaderSource_Perplexion;
    case ShaderTypeColorWheel:
      return ShaderSource_ColorWheel;
    case ShaderTypeDiscoAudio:
      return ShaderSource_DiscoAudio;
    case ShaderTypeFlickerAudio:
      return ShaderSource_FlickerAudio;
    case ShaderTypeFractalAudio:
      return ShaderSource_FractalAudio;
    case ShaderTypeUnknownPleasures:
      return ShaderSource_UnknownPleasures;
    case ShaderTypeSpaceRings:
      return ShaderSource_SpaceRings;
    case ShaderTypeCloudyShapes:
      return ShaderSource_CloudyShapes;
    case ShaderTypeAudioBlocks:
      return ShaderSource_AudioBlocks;
    case ShaderTypeFloatingSparks:
      return ShaderSource_FloatingSparks;
    case ShaderTypeCosmos:
      return ShaderSource_Cosmos;
    case ShaderTypeChromeGrill:
      return ShaderSource_ChromeGrill;
    case ShaderTypeFibers:
      return ShaderSource_Fibers;
    case ShaderTypeMist:
      return ShaderSource_Mist;
    case ShaderTypeWelcomeRings:
      return ShaderSource_WelcomeRings;
    case ShaderTypeMotionBlurTexture:
      return ShaderSource_MotionBlurTexture;
    case ShaderTypeIsoFract:
      return ShaderSource_IsoFract;
    case ShaderTypeDirtyPlasma:
      return ShaderSource_DirtyPlasma;
    case ShaderTypeTwistedTrip:
      return ShaderSource_TwistedTrip;
    case ShaderTypeTwistedCubes:
      return ShaderSource_TwistedCubes;
    case ShaderTypeCore:
      return ShaderSource_Core;
    case ShaderTypeWarpspeed:
      return ShaderSource_Warpspeed;
    case ShaderTypeReflector:
      return ShaderSource_Reflector;
    case ShaderTypeFullHouse:
      return ShaderSource_FullHouse;
    case ShaderTypeGridRun:
      return ShaderSource_GridRun;
    case ShaderTypeGyroids:
      return ShaderSource_Gyroids;
    case ShaderTypeSwirlingSoul:
      return ShaderSource_SwirlingSoul;
    case ShaderTypeSmokeRing:
      return ShaderSource_SmokeRing;
    case ShaderTypeLimbo:
      return ShaderSource_Limbo;
    case ShaderTypeVertex:
      return ShaderSource_Vertex;
    case ShaderTypeSolidColor:
      return ShaderSource_SolidColor;
    case ShaderTypeHilbert:
      return ShaderSource_Hilbert;
    case ShaderTypeWarp:
      return ShaderSource_Warp;
    case ShaderTypeFrequencyVisualizer:
      return ShaderSource_FrequencyVisualizer;
    case ShaderTypeCurlySquares:
      return ShaderSource_CurlySquares;
    case ShaderTypePlasmaTwo:
      return ShaderSource_PlasmaTwo;
    case ShaderTypeDancingSquares:
      return ShaderSource_DancingSquares;
    case ShaderTypeTissue:
      return ShaderSource_Tissue;
    case ShaderTypePsycurves:
      return ShaderSource_Psycurves;
    case ShaderTypeTriangleMap:
      return ShaderSource_TriangleMap;
    case ShaderTypeDisco:
      return ShaderSource_Disco;
    case ShaderTypeOctahedron:
      return ShaderSource_Octahedron;
    case ShaderTypeRubiks:
      return ShaderSource_Rubiks;
    case ShaderTypeMountains:
      return ShaderSource_Mountains;
    case ShaderTypeNone:
      return ShaderSource_empty;
    case ShaderTypePlasma:
      return ShaderSource_plasma;
    case ShaderTypeFractal:
      return ShaderSource_fractal;
    case ShaderTypeFuji:
      return ShaderSource_fuji;
    case ShaderTypeClouds:
      return ShaderSource_clouds;
    case ShaderTypeMelter:
      return ShaderSource_melter;
    case ShaderTypeRings:
      return ShaderSource_rings;
    case ShaderTypeAudioWaveform:
      return ShaderSource_audioWaveform;
    case ShaderTypeAudioBumper:
      return ShaderSource_audioBumper;
    case ShaderTypeAudioMountains:
      return ShaderSource_audioMountains;
    case ShaderTypeGalaxy:
      return ShaderSource_galaxy;
    default:
      return ShaderSource_empty; // Default case
  }
}

static std::string shaderSourceTypeCategory(ShaderSourceType nameType) {
  switch (nameType) {
    // Audio Reactive
    case ShaderSource_audioBumper:
    case ShaderSource_audioWaveform:
    case ShaderSource_audioMountains:
    case ShaderSource_FrequencyVisualizer:
    case ShaderSource_AudioBlocks:
    case ShaderSource_FractalAudio:
    case ShaderSource_FlickerAudio:
    case ShaderSource_DiscoAudio:
    case ShaderSource_PixelAudioParty:
    case ShaderSource_AudioBall:
    case ShaderSource_AudioGlowBars:
    case ShaderSource_AudioGraph:
    case ShaderSource_AudioOscillator:
      return "Audio Reactive";
      
      // Particles
    case ShaderSource_empty:
    case ShaderSource_ColorWheel:
    case ShaderSource_FullHouse:
    case ShaderSource_FloatingSparks:
    case ShaderSource_ColoredDrops:
    case ShaderSource_SpaceRings:
    case ShaderSource_Bubbles:
    case ShaderSource_ElectricEels:
    case ShaderSource_WelcomeRings:
      return "Particles";
      
      // Scenic
    case ShaderSource_Gyroids:
    case ShaderSource_TriangleMap: // Name
    case ShaderSource_Mountains: // Name
    case ShaderSource_clouds:
    case ShaderSource_galaxy:
    case ShaderSource_fuji:
    case ShaderSource_Vertex:
    case ShaderSource_Warpspeed:
    case ShaderSource_Core:
    case ShaderSource_Limbo:
    case ShaderSource_MotionBlurTexture:
    case ShaderSource_CloudyShapes:
    case ShaderSource_Companions:
    case ShaderSource_Flaming:
    case ShaderSource_Glassy:
      return "Scenic";
      
      // Textured
    case ShaderSource_ChromeGrill:
    case ShaderSource_GridRun:
    case ShaderSource_Fibers:
    case ShaderSource_PlasmaTwo:
    case ShaderSource_plasma:
    case ShaderSource_Warp:
    case ShaderSource_IsoFract:
    case ShaderSource_Octagrams:
    case ShaderSource_fractal:
    case ShaderSource_Cosmos:
    case ShaderSource_Breathe:
    case ShaderSource_Heptagons:
    case ShaderSource_Plasmor:
      return "Textured";
      
    // Shapes & Solids
    case ShaderSource_SolidColor: 
    case ShaderSource_SimpleBars:
    case ShaderSource_Octahedron:
    case ShaderSource_Spiral:
    case ShaderSource_SimpleShape:
    case ShaderSource_Lava:
      return "Shapes & Solids";
      
      // Psychedelic
    case ShaderSource_Mist:
    case ShaderSource_UnknownPleasures:
    case ShaderSource_SmokeRing:
    case ShaderSource_Perplexion:
    case ShaderSource_SwirlingSoul: // Name
    case ShaderSource_Hilbert: // Name
    case ShaderSource_CurlySquares: // Name
    case ShaderSource_DancingSquares: // Name
    case ShaderSource_Tissue: // Name
    case ShaderSource_Psycurves: // Name
    case ShaderSource_Disco: // Name
    case ShaderSource_Rubiks: // Name
    case ShaderSource_rings:
    case ShaderSource_melter:
    case ShaderSource_TwistedCubes:
    case ShaderSource_Reflector:
    case ShaderSource_DirtyPlasma:
    case ShaderSource_TwistedTrip:
    case ShaderSource_Aerogel:
    case ShaderSource_Diffractor:
    case ShaderSource_Accretion:
    case ShaderSource_Synthfly:
    case ShaderSource_Synthwave:
      return "Psychedelic";
    default:
      return "Unknown";
  }
};

static std::string shaderSourceTypeName(ShaderSourceType nameType) {
  return shaderTypeName(shaderTypeForShaderSourceType(nameType));
};

struct ShaderSource : public VideoSource {
public:
  ofShader maskShader;
  std::shared_ptr<Shader> shader;
  ShaderSourceType shaderSourceType;
  std::shared_ptr<ofFbo> canvas;
  
  ShaderSource(std::string id, ShaderSourceType type)
  : shader(nullptr), shaderSourceType(type),
  canvas(std::make_shared<ofFbo>()),
  VideoSource(id, shaderSourceTypeName(type), VideoSource_shader) {
    addShader(type);
  };
  
  void addShader(ShaderSourceType addType) {
    switch (addType) {
    // Shader Settings
    case ShaderSource_AudioGraph: { // Settings
      auto settings = new AudioGraphSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioGraphShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_AudioOscillator: { // Settings
      auto settings = new AudioOscillatorSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioOscillatorShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Lava: { // Settings
      auto settings = new LavaSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<LavaShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_AudioGlowBars: { // Settings
      auto settings = new AudioGlowBarsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioGlowBarsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_AudioBall: { // Settings
      auto settings = new AudioBallSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioBallShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Bubbles: { // Settings
      auto settings = new BubblesSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<BubblesShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_ElectricEels: { // Settings
      auto settings = new ElectricEelsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<ElectricEelsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Synthfly: { // Settings
        auto settings = new SynthflySettings(UUID::generateUUID(), 0);
        shader = std::make_shared<SynthflyShader>(settings);
        shader->setup();
        return;
    }
    case ShaderSource_Glassy: { // Settings
      auto settings = new GlassySettings(UUID::generateUUID(), 0);
      shader = std::make_shared<GlassyShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Synthwave: { // Settings
      auto settings = new SynthwaveSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<SynthwaveShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_GlowBeat: { // Settings
      auto settings = new GlowBeatSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<GlowBeatShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Flaming: { // Settings
      auto settings = new FlamingSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<FlamingShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Accretion: { // Settings
      auto settings = new AccretionSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AccretionShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Fireflies: { // Settings
      auto settings = new FirefliesSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<FirefliesShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_CloudyTunnel: { // Settings
      auto settings = new CloudyTunnelSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<CloudyTunnelShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_PixelAudioParty: { // Settings
      auto settings = new PixelAudioPartySettings(UUID::generateUUID(), 0);
      shader = std::make_shared<PixelAudioPartyShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Companions: { // Settings
      auto settings = new CompanionsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<CompanionsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Spiral: { // Settings
      auto settings = new SpiralSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<SpiralShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_SimpleBars: { // Settings
      auto settings = new SimpleBarsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<SimpleBarsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_SimpleShape: { // Settings
      auto settings = new SimpleShapeSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<SimpleShapeShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_ColoredDrops: { // Settings
      auto settings = new ColoredDropsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<ColoredDropsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Plasmor: { // Settings
      auto settings = new PlasmorSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<PlasmorShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Diffractor: { // Settings
      auto settings = new DiffractorSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<DiffractorShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Aerogel: { // Settings
      auto settings = new AerogelSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AerogelShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Breathe: { // Settings
      auto settings = new BreatheSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<BreatheShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Heptagons: { // Settings
      auto settings = new HeptagonsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<HeptagonsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Octagrams: { // Settings
      auto settings = new OctagramsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<OctagramsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Perplexion: { // Settings
      auto settings = new PerplexionSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<PerplexionShader>(settings);
      shader->setup();
      return;
    }
      case ShaderSource_ColorWheel: { // Settings
        auto settings = new ColorWheelSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<ColorWheelShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_DiscoAudio: { // Settings
        auto settings = new DiscoAudioSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<DiscoAudioShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_FlickerAudio: { // Settings
        auto settings = new FlickerAudioSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<FlickerAudioShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_FractalAudio: { // Settings
        auto settings = new FractalAudioSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<FractalAudioShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_UnknownPleasures: { // Settings
        auto settings = new UnknownPleasuresSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<UnknownPleasuresShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_SpaceRings: { // Settings
        auto settings = new SpaceRingsSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<SpaceRingsShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_CloudyShapes: { // Settings
        auto settings = new CloudyShapesSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<CloudyShapesShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_AudioBlocks: { // Settings
        auto settings = new AudioBlocksSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<AudioBlocksShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_FloatingSparks: { // Settings
        auto settings = new FloatingSparksSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<FloatingSparksShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Cosmos: { // Settings
        auto settings = new CosmosSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<CosmosShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_ChromeGrill: { // Settings
        auto settings = new ChromeGrillSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<ChromeGrillShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Fibers: { // Settings
        auto settings = new FibersSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<FibersShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Mist: { // Settings
        auto settings = new MistSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<MistShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_WelcomeRings: { // Settings
        auto settings = new WelcomeRingsSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<WelcomeRingsShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_MotionBlurTexture: { // Settings
        auto settings = new MotionBlurTextureSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<MotionBlurTextureShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_IsoFract: { // Settings
        auto settings = new IsoFractSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<IsoFractShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_DirtyPlasma: { // Settings
        auto settings = new DirtyPlasmaSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<DirtyPlasmaShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_TwistedTrip: { // Settings
        auto settings = new TwistedTripSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<TwistedTripShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_TwistedCubes: { // Settings
        auto settings = new TwistedCubesSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<TwistedCubesShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Core: { // Settings
        auto settings = new CoreSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<CoreShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Warpspeed: { // Settings
        auto settings = new WarpspeedSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<WarpspeedShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Reflector: { // Settings
        auto settings = new ReflectorSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<ReflectorShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_FullHouse: { // Settings
        auto settings = new FullHouseSettings(UUID::generateUUID(), 0);
        shader = std::make_shared<FullHouseShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_GridRun: { // Settings
        auto settings = new GridRunSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<GridRunShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Gyroids: { // Settings
        auto settings = new GyroidsSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<GyroidsShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_SwirlingSoul: { // Settings
        auto settings = new SwirlingSoulSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<SwirlingSoulShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_SmokeRing: { // Settings
        auto settings = new SmokeRingSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<SmokeRingShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Limbo: { // Settings
        auto settings = new LimboSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<LimboShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Vertex: { // Settings
        auto settings = new VertexSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<VertexShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_SolidColor: { // Settings
        auto settings = new SolidColorSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<SolidColorShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Hilbert: { // Settings
        auto settings = new HilbertSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<HilbertShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Warp: { // Settings
        auto settings = new WarpSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<WarpShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_FrequencyVisualizer: { // Settings
        auto settings = new FrequencyVisualizerSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<FrequencyVisualizerShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_CurlySquares: { // Settings
        auto settings = new CurlySquaresSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<CurlySquaresShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_PlasmaTwo: { // Settings
        auto settings = new PlasmaTwoSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<PlasmaTwoShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_DancingSquares: { // Settings
        auto settings = new DancingSquaresSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<DancingSquaresShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Tissue: { // Settings
        auto settings = new TissueSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<TissueShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Psycurves: { // Settings
        auto settings = new PsycurvesSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<PsycurvesShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_TriangleMap: { // Settings
        auto settings = new TriangleMapSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<TriangleMapShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Disco: { // Settings
        auto settings = new DiscoSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<DiscoShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Octahedron: { // Settings
        auto settings = new OctahedronSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<OctahedronShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Rubiks: { // Settings
        auto settings = new RubiksSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<RubiksShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_Mountains: { // Settings
        auto settings = new MountainsSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<MountainsShader>(settings);
        shader->setup();
        return;
      }
      case ShaderSource_empty: {
        auto plasmaSettings = new EmptySettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<EmptyShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_audioMountains: {
        auto plasmaSettings = new AudioMountainsSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<AudioMountainsShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_galaxy: {
        auto plasmaSettings = new GalaxySettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<GalaxyShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_audioBumper: {
        auto plasmaSettings = new AudioBumperSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<AudioBumperShader>(plasmaSettings);
        shader->setup();
        return;
      }
        
      case ShaderSource_audioWaveform: {
        auto plasmaSettings = new AudioWaveformSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<AudioWaveformShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_rings: {
        auto plasmaSettings = new RingsSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<RingsShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_plasma: {
        auto plasmaSettings = new PlasmaSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<PlasmaShader>(plasmaSettings);
        shader->setup();
        return;
      }
      case ShaderSource_fractal: {
        auto fractalSettings = new FractalSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<FractalShader>(fractalSettings);
        shader->setup();
        return;
      }
      case ShaderSource_fuji: {
        auto fujiSettings = new FujiSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<FujiShader>(fujiSettings);
        shader->setup();
        return;
      }
      case ShaderSource_clouds: {
        auto cloudsSettings = new CloudSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<CloudShader>(cloudsSettings);
        shader->setup();
        return;
      }
      case ShaderSource_melter: {
        auto melterSettings = new MeltSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
        shader = std::make_shared<MeltShader>(melterSettings);
        shader->setup();
        return;
      }
    }
  }
  
  void setup() override;
  
  void saveFrame() override {
    // If our width or height has changed, setup again
    if (fbo->getWidth() != LayoutStateService::getService()->resolution.x ||
        fbo->getHeight() != LayoutStateService::getService()->resolution.y) {
      setup();
    }
    
    canvas->begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    canvas->end();
    
    // Check for file changes
    shader->checkForFileChanges();
    
    shader->shade(fbo, canvas);
    shader->activateParameters();
    
    fbo->begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    canvas->draw(0, 0, fbo->getWidth(), fbo->getHeight());
    fbo->end();
    
  }
  
  void drawSettings() override {
    if (shader != nullptr) {
      shader->drawSettings();
    }
  }
  
  void draw();
  
  json serialize() override;
};

#endif /* ShaderSource_hpp */
