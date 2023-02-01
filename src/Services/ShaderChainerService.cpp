//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"
#include "AsciiShader.hpp"
#include "SobelShader.hpp"
#include "TriangleMapShader.hpp"
#include "LiquidShader.hpp"
#include "DiscoShader.hpp"
#include "OctahedronShader.hpp"
#include "AudioBumperShader.hpp"
#include "AvailableShader.hpp"
#include "AudioMountainsShader.hpp"
#include "AudioWaveformShader.hpp"
#include "BlurShader.hpp"
#include "CloudShader.hpp"
#include "ConfigService.hpp"
#include "Console.hpp"
#include "DitherShader.hpp"
#include "FeedbackShader.hpp"
#include "GalaxyShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "KaleidoscopeShader.hpp"
#include "MelterShader.hpp"
#include "MirrorShader.hpp"
#include "MixShader.hpp"
#include "MountainsShader.hpp"
#include "PixelShader.hpp"
#include "RGBShiftShader.hpp"
#include "RainbowRotatorShader.hpp"
#include "RingsShader.hpp"
#include "RubiksShader.hpp"
#include "ShaderChainerService.hpp"
#include "SliderShader.hpp"
#include "TileShader.hpp"
#include "TransformShader.hpp"
#include "VanGoghShader.hpp"
#include "VideoSourceService.hpp"
#include "WobbleShader.hpp"

void ShaderChainerService::setup() {
  // Create an AvailableShader for each type
  for (auto const shaderType : AvailableShaderTypes) {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableShaders.push_back(shader);
  }
}

std::vector<std::shared_ptr<ShaderChainer>>
ShaderChainerService::shaderChainers() {
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers;
  for (auto const &[key, val] : shaderChainerMap) {
    shaderChainers.push_back(val);
  }
  // Sort the ShaderChainers by their names
  std::sort(shaderChainers.begin(), shaderChainers.end(),
            [](const std::shared_ptr<ShaderChainer> &a,
               const std::shared_ptr<ShaderChainer> &b) {
              return a->creationTime < b->creationTime;
            });

  return shaderChainers;
}

std::vector<std::string> ShaderChainerService::shaderChainerNames() {
  std::vector<std::string> names;
  for (auto const &[key, val] : shaderChainerMap) {
    names.push_back(val->name);
  }
  return names;
}

void ShaderChainerService::updateShaderChainers() {
  for (auto const &[key, val] : shaderChainerMap) {
    val->update();
  }
}

void ShaderChainerService::subscribeToShaderChainerUpdates(
    std::function<void()> callback) {
  shaderChainerUpdateSubject.subscribe(callback);
}

void ShaderChainerService::removeShaderChainer(std::string id) {
  if (shaderChainerMap.count(id) == 0) {
    log("Tried to remove ShaderChainer %s, but it doesn't exist", id.c_str());
    return;
  }
  shaderChainerMap.erase(id);
  shaderChainerUpdateSubject.notify();
}

void ShaderChainerService::selectShaderChainer(
    ::shared_ptr<ShaderChainer> shaderChainer) {
  selectedShaderChainer = shaderChainer;
  // When we select a ShaderChainer, we deselect the Shader
  selectedShader = nullptr;
}

void ShaderChainerService::selectShader(std::shared_ptr<Shader> shader) {
  selectedShader = shader;
}

void ShaderChainerService::breakShaderAuxLink(
    std::shared_ptr<Shader> startShader, std::shared_ptr<Shader> endShader) {
  startShader->next = nullptr;
  endShader->aux = nullptr;
}

void ShaderChainerService::addNewShaderChainer(
    std::shared_ptr<VideoSource> videoSource) {
  auto name = formatString("%s-Chainer", videoSource->sourceName.c_str());
  auto chainer =
      std::make_shared<ShaderChainer>(UUID::generateUUID(), name, videoSource);
  addShaderChainer(chainer);
}

void ShaderChainerService::addShaderChainer(
    std::shared_ptr<ShaderChainer> shaderChainer) {
  if (shaderChainerMap.count(shaderChainer->chainerId) != 0) {
    log("Reregistering ShaderChainer %s", shaderChainer->chainerId.c_str());
  }

  // Select the first ShaderChainer to be created
  if (shaderChainerMap.empty()) {
    selectShaderChainer(shaderChainer);
  }

  shaderChainerMap[shaderChainer->chainerId] = shaderChainer;

  VideoSourceService::getService()->addVideoSource(shaderChainer);
  shaderChainerUpdateSubject.notify();
}

std::shared_ptr<Shader> ShaderChainerService::makeShader(ShaderType type) {
  auto shader = shaderForType(type, UUID::generateUUID(), 0);
  addShader(shader);
  return shader;
}

void ShaderChainerService::linkShaderToNext(
    std::shared_ptr<Shader> destShader, std::shared_ptr<Shader> sourceShader) {
  sourceShader->next = destShader;
  destShader->parent = sourceShader;
}

void ShaderChainerService::addShaderToFront(
    std::shared_ptr<Shader> destShader,
    std::shared_ptr<ShaderChainer> chainer) {
  chainer->front = destShader;
}

// ConfigurableService

json ShaderChainerService::config() {
  json container;

  for (auto &pair : shaderChainerMap) {
    container[pair.first] = pair.second->serialize();
  }

  return container;
}

void ShaderChainerService::loadConfig(json data) {
  shaderChainerMap.clear();

  if (data.is_object()) {
    std::map<std::string, json> items = data;

    for (auto const &pair : items) {
      std::string chainerName = pair.second["name"];
      std::string sourceId = pair.second["sourceId"];

      auto source =
          VideoSourceService::getService()->videoSourceForId(sourceId);
      if (source == nullptr) {
        log("Failed to find VideoSource for %s", sourceId.c_str());
        return;
      }

      auto shaderChainer = new ShaderChainer(pair.first, chainerName, source);
      shaderChainer->load(pair.second);
      shaderChainer->setup();
      addShaderChainer(std::shared_ptr<ShaderChainer>(shaderChainer));
    }
  }
}

void ShaderChainerService::addShader(std::shared_ptr<Shader> shader) {
  if (shadersMap.count(shader->shaderId) != 0) {
    log("Reregistering Shader %s", shader->shaderId.c_str());
  }
  if (shader == nullptr) {
    log("Tried to add a null Shader");
    return;
  }

  shadersMap[shader->id()] = shader;
}

void ShaderChainerService::removeShader(std::shared_ptr<Shader> shader) {
  if (shadersMap.count(shader->shaderId) == 0) {
    log("Tried to remove Shader %s, but it doesn't exist",
        shader->shaderId.c_str());
    return;
  }

  // If the Shader is at the front of out chainer, remove that connection
  if (shaderChainerForShaderId(shader->shaderId) != nullptr) {
    auto chainer = shaderChainerForShaderId(shader->shaderId);
    if (chainer->front == shader) {
      chainer->front = nullptr;
    }
  }

  shadersMap.erase(shader->shaderId);
  if (shader->parent != nullptr)
    shader->parent->next = nullptr;

  shader->next = nullptr;

  shader.reset();
}

void ShaderChainerService::associateShaderWithChainer(
    std::string shaderId, std::shared_ptr<ShaderChainer> chainer) {
  shaderIdShaderChainerMap[shaderId] = chainer;
}

std::shared_ptr<Shader> ShaderChainerService::shaderForId(std::string id) {
  if (shadersMap.count(id) == 0) {
    log("Tried to get Shader %s, but it doesn't exist", id.c_str());
    return nullptr;
  }
  return shadersMap[id];
}

std::vector<std::shared_ptr<Shader>> ShaderChainerService::shaders() {
  std::vector<std::shared_ptr<Shader>> shaders;
  for (auto const &[key, val] : shadersMap) {
    shaders.push_back(val);
  }
  // Sort the shaders by creationTime
  std::sort(shaders.begin(), shaders.end(),
            [](std::shared_ptr<Shader> a, std::shared_ptr<Shader> b) {
              return a->creationTime < b->creationTime;
            });
  return shaders;
}

void ShaderChainerService::breakShaderNextLink(std::shared_ptr<Shader> shader) {
  if (shader->next != nullptr) {
    shader->next->parent = nullptr;
    shader->next = nullptr;
  }
}

void ShaderChainerService::breakShaderChainerFront(
    std::shared_ptr<ShaderChainer> shaderChainer) {
  if (shaderChainer != nullptr) {
    shaderChainer->front = nullptr;
  }
}

std::shared_ptr<ShaderChainer>
ShaderChainerService::shaderChainerForShaderId(std::string id) {
  return shaderIdShaderChainerMap[id];
}

void ShaderChainerService::setAuxShader(std::shared_ptr<Shader> auxShader,
                                        std::shared_ptr<Shader> destShader) {
  destShader->aux = auxShader;
}

std::shared_ptr<Shader>
ShaderChainerService::shaderForType(ShaderType type, std::string shaderId,
                                    json shaderJson) {
  switch (type) {
    case ShaderTypeSobel: {
      auto settings = new SobelSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SobelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTriangleMap: {
      auto settings = new TriangleMapSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TriangleMapShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLiquid: {
      auto settings = new LiquidSettings(shaderId, shaderJson);
      auto shader = std::make_shared<LiquidShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDisco: {
      auto settings = new DiscoSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DiscoShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOctahedron: {
      auto settings = new OctahedronSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OctahedronShader>(settings);
      shader->setup();
      return shader;
    }
  case ShaderTypeVanGogh: {
    auto settings = new VanGoghSettings(shaderId, shaderJson);
    auto shader = std::make_shared<VanGoghShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeRubiks: {
    auto settings = new RubiksSettings(shaderId, shaderJson);
    auto shader = std::make_shared<RubiksShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeRainbowRotator: {
    auto settings = new RainbowRotatorSettings(shaderId, shaderJson);
    auto shader = std::make_shared<RainbowRotatorShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeDither: {
    auto settings = new DitherSettings(shaderId, shaderJson);
    auto shader = std::make_shared<DitherShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeMountains: {
    auto settings = new MountainsSettings(shaderId, shaderJson);
    auto shader = std::make_shared<MountainsShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeGalaxy: {
    auto settings = new GalaxySettings(shaderId, shaderJson);
    auto shader = std::make_shared<GalaxyShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeAudioMountains: {
    auto settings = new AudioMountainsSettings(shaderId, shaderJson);
    auto shader = std::make_shared<AudioMountainsShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeAudioBumper: {
    auto settings = new AudioBumperSettings(shaderId, shaderJson);
    auto shader = std::make_shared<AudioBumperShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeAudioWaveform: {
    auto settings = new AudioWaveformSettings(shaderId, shaderJson);
    auto shader = std::make_shared<AudioWaveformShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeRings: {
    auto settings = new RingsSettings(shaderId, shaderJson);
    auto shader = std::make_shared<RingsShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeSlider: {
    auto settings = new SliderSettings(shaderId, shaderJson);
    auto shader = std::make_shared<SliderShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeWobble: {
    auto settings = new WobbleSettings(shaderId, shaderJson);
    auto shader = std::make_shared<WobbleShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeRGBShift: {
    auto settings = new RGBShiftSettings(shaderId, shaderJson);
    auto shader = std::make_shared<RGBShiftShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeNone:
    return 0;
  case ShaderTypeHSB: {
    auto settings = new HSBSettings(shaderId, shaderJson);
    auto shader = std::make_shared<HSBShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeBlur: {
    auto settings = new BlurSettings(shaderId, shaderJson);
    auto shader = std::make_shared<BlurShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypePixelate: {
    auto settings = new PixelSettings(shaderId, shaderJson);
    auto shader = std::make_shared<PixelShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeGlitch:
    return 0;
  case ShaderTypeMirror: {
    auto settings = new MirrorSettings(shaderId, shaderJson);
    auto shader = std::make_shared<MirrorShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeTransform: {
    auto settings = new TransformSettings(shaderId, shaderJson);
    auto shader = std::make_shared<TransformShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeFeedback: {
    auto settings = new FeedbackSettings(shaderId, shaderJson);
    auto shader = std::make_shared<FeedbackShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeAscii: {
    auto settings = new AsciiSettings(shaderId, shaderJson);
    auto shader = std::make_shared<AsciiShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeKaleidoscope: {
    auto settings = new KaleidoscopeSettings(shaderId, shaderJson);
    auto shader = std::make_shared<KaleidoscopeShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeTile: {
    auto settings = new TileSettings(shaderId, shaderJson);
    auto shader = std::make_shared<TileShader>(settings);
    shader->setup();
    return shader;
  }
  case ShaderTypeMix: {
    auto settings = new MixSettings(shaderId, shaderJson);
    auto shader = std::make_shared<MixShader>(settings);
    shader.get()->setup();
    return shader;
  }
  case ShaderTypePlasma: {
    auto settings = new PlasmaSettings(shaderId, shaderJson);
    auto shader = std::make_shared<PlasmaShader>(settings);
    shader.get()->setup();
    return shader;
  }
  case ShaderTypeFuji: {
    auto settings = new FujiSettings(shaderId, shaderJson);
    auto shader = std::make_shared<FujiShader>(settings);
    shader.get()->setup();
    return shader;
    break;
  }
  case ShaderTypeFractal: {
    auto settings = new FractalSettings(shaderId, shaderJson);
    auto shader = std::make_shared<FractalShader>(settings);
    shader.get()->setup();
    return shader;
    break;
  }
  case ShaderTypeClouds: {
    auto settings = new CloudSettings(shaderId, shaderJson);
    auto shader = std::make_shared<CloudShader>(settings);
    shader.get()->setup();
    return shader;
    break;
  }
  case ShaderTypeMelter: {
    auto settings = new MeltSettings(shaderId, shaderJson);
    auto shader = std::make_shared<MeltShader>(settings);
    shader.get()->setup();
    return shader;
    break;
  }
  }
}
