// clang-format off

#ifndef ShaderSource_hpp
#define ShaderSource_hpp

#include "AudioBumperShader.hpp"
#include "TissueShader.hpp"
#include "TissueShader.hpp"
#include "TissueShader.hpp"
#include "TissueShader.hpp"
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
#include "PlasmaShader.hpp"
#include "RingsShader.hpp"
#include "Shader.hpp"
#include "UUID.hpp"
#include "ShaderType.hpp"

#include "VideoSource.hpp"
#include <stdio.h>

using json = nlohmann::json;

enum ShaderSourceType {
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
  ShaderSource_galaxy
};

static const ShaderSourceType AvailableShaderSourceTypes[] = {
// Available ShaderSourceTypes
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
  ShaderSource_galaxy
};

static ShaderType shaderTypeForShaderSourceType(ShaderSourceType type) {
  switch (type) {
// shaderTypeForShaderSourceType
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

static std::string shaderSourceTypeName(ShaderSourceType nameType) {
  switch (nameType) {
  // Shader Names
  case ShaderSource_Tissue: // Name  
    return "Tissue"; // Tissue
  case ShaderSource_Psycurves: // Name  
    return "Psycurves"; // Psycurves
  case ShaderSource_TriangleMap: // Name  
    return "TriangleMap"; // TriangleMap
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
  std::shared_ptr<Shader> shader;
  ShaderSourceType shaderSourceType;
  ofFbo fbo;
  ofFbo canvas;

  ShaderSource(std::string id, ShaderSourceType type)
      : shader(nullptr), shaderSourceType(type),
        VideoSource(id, shaderSourceTypeName(type), VideoSource_shader) {
    addShader(type);
  };

  void addShader(ShaderSourceType addType) {
    switch (addType) {
    // Shader Settings
    case ShaderSource_Tissue: { // Settings
      auto settings = new TissueSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<TissueShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Psycurves: { // Settings
      auto settings = new PsycurvesSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<PsycurvesShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_TriangleMap: { // Settings
      auto settings = new TriangleMapSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<TriangleMapShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Disco: { // Settings
      auto settings = new DiscoSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<DiscoShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Octahedron: { // Settings
      auto settings = new OctahedronSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<OctahedronShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_VanGogh: { // Settings
      auto settings = new VanGoghSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<VanGoghShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Rubiks: { // Settings
      auto settings = new RubiksSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<RubiksShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_Mountains: { // Settings
      auto settings = new MountainsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<MountainsShader>(settings);
      shader->setup();
      return;
    }
    case ShaderSource_empty: {
      auto plasmaSettings = new EmptySettings(UUID::generateUUID(), 0);
      shader = std::make_shared<EmptyShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_audioMountains: {
      auto plasmaSettings = new AudioMountainsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioMountainsShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_galaxy: {
      auto plasmaSettings = new GalaxySettings(UUID::generateUUID(), 0);
      shader = std::make_shared<GalaxyShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_audioBumper: {
      auto plasmaSettings = new AudioBumperSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioBumperShader>(plasmaSettings);
      shader->setup();
      return;
    }

    case ShaderSource_audioWaveform: {
      auto plasmaSettings = new AudioWaveformSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<AudioWaveformShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_rings: {
      auto plasmaSettings = new RingsSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<RingsShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_plasma: {
      auto plasmaSettings = new PlasmaSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<PlasmaShader>(plasmaSettings);
      shader->setup();
      return;
    }
    case ShaderSource_fractal: {
      auto fractalSettings = new FractalSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<FractalShader>(fractalSettings);
      shader->setup();
      return;
    }
    case ShaderSource_fuji: {
      auto fujiSettings = new FujiSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<FujiShader>(fujiSettings);
      shader->setup();
      return;
    }
    case ShaderSource_clouds: {
      auto cloudsSettings = new CloudSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<CloudShader>(cloudsSettings);
      shader->setup();
      return;
    }
    case ShaderSource_melter: {
      auto melterSettings = new MeltSettings(UUID::generateUUID(), 0);
      shader = std::make_shared<MeltShader>(melterSettings);
      shader->setup();
      return;
    }
    }
  }

  void setup() override {
    frameTexture = std::make_shared<ofTexture>();
    frameTexture->allocate(1280, 720, GL_RGB);
    frameBuffer.bind(GL_SAMPLER_2D_RECT);
    frameBuffer.allocate(1280 * 720 * 4, GL_STATIC_COPY);

    shader->setup();
    fbo.allocate(1280, 720, GL_RGB);
    fbo.begin();
    ofClear(0, 0, 0, 255);
    fbo.end();

    canvas.allocate(1280, 720, GL_RGB);
    canvas.begin();
    ofClear(0, 0, 0, 255);
    canvas.end();
  };

  void saveFrame() override {
    shader->shade(&fbo, &canvas);
    frameTexture = std::make_shared<ofTexture>(canvas.getTexture());
  }

  void drawSettings() override {
    if (shader != nullptr) {
      shader->drawSettings();
    }
  }

  void update();
  void draw();

  json serialize() override {
    json j;
    j["videoSourceType"] = VideoSource_shader;
    j["shaderSourceType"] = shaderSourceType;
    j["id"] = id;
    return j;
  }
};

#endif /* ShaderSource_hpp */
