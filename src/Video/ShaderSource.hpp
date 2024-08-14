
// clang-format off

#ifndef ShaderSource_hpp
#define ShaderSource_hpp

#include "AudioBumperShader.hpp"
#include "StarsShader.hpp"
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
#include "VoronoiColumnsShader.hpp"
#include "WarpspeedShader.hpp"
#include "ReflectorShader.hpp"
#include "FullHouseShader.hpp"
#include "GridRunShader.hpp"
#include "GyroidsShader.hpp"
#include "SwirlingSoulShader.hpp"
#include "DoubleSwirlShader.hpp"
#include "SmokeRingShader.hpp"
#include "AudioCircleShader.hpp"
#include "LimboShader.hpp"
#include "VertexShader.hpp"
#include "SolidColorShader.hpp"
#include "HilbertShader.hpp"
#include "WarpShader.hpp"
#include "FrequencyVisualizerShader.hpp"
#include "CurlySquaresShader.hpp"
#include "PlasmaTwoShader.hpp"
#include "DancingSquaresShader.hpp"
#include "CircleShader.hpp"
#include "TissueShader.hpp"
#include "PsycurvesShader.hpp"
#include "TriangleMapShader.hpp"
#include "DiscoShader.hpp"
#include "OctahedronShader.hpp"
#include "VanGoghShader.hpp"
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
  ShaderSource_FrequencyVisualizer, //source enum,
  ShaderSource_CurlySquares, //source enum,
  ShaderSource_PlasmaTwo, //source enum,
  ShaderSource_DancingSquares, //source enum,
  ShaderSource_Circle, //source enum,
  ShaderSource_Tissue, //source enum,
  ShaderSource_Psycurves,
  ShaderSource_TriangleMap,
  ShaderSource_Disco,
  ShaderSource_Octahedron,
  ShaderSource_VanGogh,
  ShaderSource_Rubiks,
  ShaderSource_Mountains,
  ShaderSource_empty,
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
  ShaderSource_Warp, //source enum,
  ShaderSource_Hilbert, //source enum,
  ShaderSource_SolidColor, //source enum,
  ShaderSource_Vertex, //source enum,
  ShaderSource_Limbo, //source enum,
  ShaderSource_AudioCircle, //source enum,
  ShaderSource_SmokeRing, //source enum,
  ShaderSource_DoubleSwirl, //source enum,
  ShaderSource_SwirlingSoul, //source enum,
  ShaderSource_Gyroids, //source enum,
  ShaderSource_GridRun, //source enum,
  ShaderSource_FullHouse, //source enum,
  ShaderSource_Reflector, //source enum,
  ShaderSource_Warpspeed, //source enum,
  ShaderSource_VoronoiColumns, //source enum,
  ShaderSource_Core, //source enum,
  ShaderSource_TwistedCubes, //source enum,
  ShaderSource_TwistedTrip, //source enum,
  ShaderSource_DirtyPlasma, //source enum,
  ShaderSource_IsoFract, //source enum,
  ShaderSource_MotionBlurTexture, //source enum,
  ShaderSource_WelcomeRings, //source enum,
  ShaderSource_Mist, //source enum,
  ShaderSource_Fibers, //source enum,
  ShaderSource_ChromeGrill, //source enum,
  ShaderSource_GlitchAudio, //source enum,
  ShaderSource_Stars, //source enum,
}; // End ShaderSourceType

static const ShaderSourceType AvailableShaderSourceTypes[] = {
// Available ShaderSourceTypes
  ShaderSource_FrequencyVisualizer, // Available
  ShaderSource_CurlySquares, // Available
  ShaderSource_PlasmaTwo, // Available
  ShaderSource_DancingSquares, // Available
  ShaderSource_Circle, // Available
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
  ShaderSource_Hilbert, // Available
  ShaderSource_SolidColor, // Available
  ShaderSource_Vertex, // Available
  ShaderSource_Limbo, // Available
  ShaderSource_SmokeRing, // Available
  ShaderSource_DoubleSwirl, // Available
  ShaderSource_SwirlingSoul, // Available
  ShaderSource_Gyroids, // Available
  ShaderSource_GridRun, // Available
  ShaderSource_FullHouse, // Available
  ShaderSource_Reflector, // Available
  ShaderSource_Warpspeed, // Available
  ShaderSource_Core, // Available
  ShaderSource_TwistedCubes, // Available
  ShaderSource_TwistedTrip, // Available
  ShaderSource_DirtyPlasma, // Available
  ShaderSource_IsoFract, // Available
  ShaderSource_MotionBlurTexture, // Available
  ShaderSource_WelcomeRings, // Available
  ShaderSource_Mist, // Available
  ShaderSource_Fibers, // Available
  ShaderSource_ChromeGrill, // Available
  ShaderSource_GlitchAudio, // Available
  ShaderSource_Stars, // Available
}; // End AvailableShaderSourceTypes

static ShaderType shaderTypeForShaderSourceType(ShaderSourceType type) {
  switch (type) {
// shaderTypeForShaderSourceType
  case ShaderSource_Stars: //type enum
    return ShaderTypeStars;
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
  case ShaderSource_VoronoiColumns: //type enum
    return ShaderTypeVoronoiColumns;
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
  case ShaderSource_DoubleSwirl: //type enum
    return ShaderTypeDoubleSwirl;
  case ShaderSource_SmokeRing: //type enum
    return ShaderTypeSmokeRing;
  case ShaderSource_AudioCircle: //type enum
    return ShaderTypeAudioCircle;
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
  case ShaderSource_Circle: //type enum
    return ShaderTypeCircle;
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
  case ShaderSource_VanGogh:
    return ShaderTypeVanGogh;
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

static std::string shaderSourceTypeCategory(ShaderSourceType nameType) {
  switch (nameType) {
  // Audio Reactive
    case ShaderSource_audioBumper:
    case ShaderSource_audioWaveform:
    case ShaderSource_audioMountains:
    case ShaderSource_FrequencyVisualizer:
    case ShaderSource_VoronoiColumns:
      return "Audio Reactive";
    
  // Simple
    case ShaderSource_SolidColor: // Name
    case ShaderSource_Circle: // Name
    case ShaderSource_Octahedron: // Name
    case ShaderSource_empty:
      return "Simple";
      
  // Scenic
  case ShaderSource_Gyroids:
  case ShaderSource_TriangleMap: // Name
  case ShaderSource_VanGogh: // Name
  case ShaderSource_Mountains: // Name
  case ShaderSource_clouds:
  case ShaderSource_galaxy:
  case ShaderSource_fuji:
  case ShaderSource_Vertex:
  case ShaderSource_Warpspeed:
  case ShaderSource_Core:
    return "Scenic";
    
  // Glitchy
    
  // Trippy
  case ShaderSource_IsoFract:
  case ShaderSource_SwirlingSoul: // Name
  case ShaderSource_Hilbert: // Name
  case ShaderSource_Warp: // Name
  case ShaderSource_CurlySquares: // Name
  case ShaderSource_PlasmaTwo: // Name
  case ShaderSource_DancingSquares: // Name
  case ShaderSource_Tissue: // Name
  case ShaderSource_Psycurves: // Name
  case ShaderSource_Disco: // Name
  case ShaderSource_Rubiks: // Name
  case ShaderSource_rings:
  case ShaderSource_melter:
  case ShaderSource_plasma:
  case ShaderSource_TwistedCubes:
  case ShaderSource_fractal:
  case ShaderSource_Reflector:
    case ShaderSource_TwistedTrip:
    return "Psychedelic";
  default:
    return "Unknown";
  }
};

static std::string shaderSourceTypeName(ShaderSourceType nameType) {
  switch (nameType) { // ShaderNames
  case ShaderSource_Stars: // Name  
    return "Stars"; // Stars
  case ShaderSource_GlitchAudio: // Name  
    return "GlitchAudio"; // GlitchAudio
  case ShaderSource_ChromeGrill: // Name  
    return "ChromeGrill"; // ChromeGrill
  case ShaderSource_Fibers: // Name  
    return "Fibers"; // Fibers
  case ShaderSource_Mist: // Name  
    return "Mist"; // Mist
  case ShaderSource_WelcomeRings: // Name  
    return "WelcomeRings"; // WelcomeRings
  case ShaderSource_MotionBlurTexture: // Name  
    return "MotionBlurTexture"; // MotionBlurTexture
  case ShaderSource_IsoFract: // Name
    return "IsoFract"; // IsoFract
  case ShaderSource_DirtyPlasma: // Name
    return "DirtyPlasma"; // DirtyPlasma
  case ShaderSource_TwistedTrip: // Name  
    return "TwistedTrip"; // TwistedTrip
  case ShaderSource_TwistedCubes: // Name  
    return "TwistedCubes"; // TwistedCubes
  case ShaderSource_Core: // Name  
    return "Core"; // Core
  case ShaderSource_VoronoiColumns: // Name  
    return "VoronoiColumns"; // VoronoiColumns
  case ShaderSource_Warpspeed: // Name  
    return "Warpspeed"; // Warpspeed
  case ShaderSource_Reflector: // Name  
    return "Reflector"; // Reflector
  case ShaderSource_FullHouse: // Name  
    return "FullHouse"; // FullHouse
  case ShaderSource_GridRun: // Name  
    return "GridRun"; // GridRun
  case ShaderSource_Gyroids: // Name  
    return "Gyroids"; // Gyroids
  case ShaderSource_SwirlingSoul: // Name  
    return "Swirling Soul"; // SwirlingSoul
  case ShaderSource_DoubleSwirl: // Name
    return "Double Swirl"; // DoubleSwirl
  case ShaderSource_SmokeRing: // Name
    return "Smoke Ring"; // SmokeRing
  case ShaderSource_AudioCircle: // Name
    return "Audio Circle"; // AudioCircle
  case ShaderSource_Limbo: // Name
    return "Limbo"; // Limbo
  case ShaderSource_Vertex: // Name  
    return "Vertex"; // Vertex
  case ShaderSource_SolidColor: // Name  
    return "Solid Color"; // SolidColor
  case ShaderSource_Hilbert: // Name
    return "Hilbert"; // Hilbert
  case ShaderSource_Warp: // Name  
    return "Warp"; // Warp
  case ShaderSource_FrequencyVisualizer: // Name  
    return "Frequency Visualizer"; // FrequencyVisualizer
  case ShaderSource_CurlySquares: // Name
    return "Curly Squares"; // CurlySquares
  case ShaderSource_PlasmaTwo: // Name
    return "Plasma Two"; // PlasmaTwo
  case ShaderSource_DancingSquares: // Name
    return "Dancing Squares"; // DancingSquares
  case ShaderSource_Circle: // Name
    return "Circle"; // Circle
  case ShaderSource_Tissue: // Name  
    return "Tissue"; // Tissue
  case ShaderSource_Psycurves: // Name  
    return "Psycurves"; // Psycurves
  case ShaderSource_TriangleMap: // Name  
    return "Triangle Map"; // TriangleMap
  case ShaderSource_Disco: // Name
    return "Disco"; // Disco
  case ShaderSource_Octahedron: // Name  
    return "Octahedron"; // Octahedron
  case ShaderSource_VanGogh: // Name  
    return "VanGogh"; // VanGogh
  case ShaderSource_Rubiks: // Name  
    return "Rubiks"; // Rubiks
  case ShaderSource_Mountains: // Name
    return "Mountains";        // Mountain
  case ShaderSource_empty:
    return "Empty";
  case ShaderSource_plasma:
    return "Plasma";
  case ShaderSource_fractal:
    return "Fractal";
  case ShaderSource_fuji:
    return "Fuji";
  case ShaderSource_clouds:
    return "Clouds";
  case ShaderSource_melter:
    return "Melter";
  case ShaderSource_rings:
    return "Rings";
  case ShaderSource_audioWaveform:
    return "Audio Waveform";
  case ShaderSource_audioBumper:
    return "Audio Bumper";
  case ShaderSource_audioMountains:
    return "Audio Mountains";
  case ShaderSource_galaxy:
    return "Galaxy";
  default:
    return "Unknown";
  }
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
    case ShaderSource_Stars: { // Settings
      auto settings = new StarsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<StarsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_GlitchAudio: { // Settings
      auto settings = new GlitchAudioSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<GlitchAudioShader>(settings);
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
    case ShaderSource_VoronoiColumns: { // Settings
      auto settings = new VoronoiColumnsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<VoronoiColumnsShader>(settings);
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
    case ShaderSource_DoubleSwirl: { // Settings
      auto settings = new DoubleSwirlSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
      shader = std::make_shared<DoubleSwirlShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_SmokeRing: { // Settings
      auto settings = new SmokeRingSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
      shader = std::make_shared<SmokeRingShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_AudioCircle: { // Settings
      auto settings = new AudioCircleSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
      shader = std::make_shared<AudioCircleShader>(settings);
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
    case ShaderSource_Circle: { // Settings
      auto settings = new CircleSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
      shader = std::make_shared<CircleShader>(settings);
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
    case ShaderSource_VanGogh: { // Settings
      auto settings = new VanGoghSettings(UUID::generateUUID(), 0, shaderSourceTypeName(addType));
      shader = std::make_shared<VanGoghShader>(settings);
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
    ofClear(0,0,0, 255);
    canvas->end();
    
    shader->shade(fbo, canvas);
    shader->activateParameters();
    
    if (settings->maskEnabled->boolValue == true) {
      fbo->begin();
      maskShader.begin();
      maskShader.setUniformTexture("tex", canvas->getTexture(), 0);
      maskShader.setUniform1f("time", ofGetElapsedTimef());
      maskShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
      maskShader.setUniform1i("drawTex", 1);
      maskShader.setUniform4f("chromaKey",
                              settings->maskColor->color->data()[0],
                              settings->maskColor->color->data()[1],
                              settings->maskColor->color->data()[2], 1.0);
      maskShader.setUniform1f("tolerance", settings->maskTolerance->value);
      maskShader.setUniform1i("invert", settings->invert->boolValue);
      ofClear(0, 0, 0, 255);
      ofClear(0, 0, 0, 0);
      
      canvas->draw(0, 0, fbo->getWidth(), fbo->getHeight());
      maskShader.end();
      fbo->end();
    } else {
      fbo->begin();
      ofClear(0, 0, 0, 255);
      canvas->draw(0, 0, fbo->getWidth(), fbo->getHeight());
      fbo->end();
    }
  }

  void drawSettings() override {
    if (shader != nullptr) {
      shader->drawSettings();
      drawMaskSettings();
    }
  }

  void draw();
  
  json serialize();
};

#endif /* ShaderSource_hpp */
