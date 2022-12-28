//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"
#include "AsciiShader.hpp"
#include "AudioMountainsShader.hpp"
#include "AudioBumperShader.hpp"
#include "AudioWaveformShader.hpp"
#include "RingsShader.hpp"
#include "BlurShader.hpp"
#include "ConfigService.hpp"
#include "Console.hpp"
#include "FeedbackShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "KaleidoscopeShader.hpp"
#include "MirrorShader.hpp"
#include "MixShader.hpp"
#include "PixelShader.hpp"
#include "RGBShiftShader.hpp"
#include "ShaderChainerService.hpp"
#include "SliderShader.hpp"
#include "TileShader.hpp"
#include "TransformShader.hpp"
#include "VideoSourceService.hpp"
#include "WobbleShader.hpp"

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
              return a->name < b->name;
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

int ShaderChainerService::count() { return shaderChainerMap.size(); }

void ShaderChainerService::selectShaderChainer(
    ::shared_ptr<ShaderChainer> shaderChainer) {
  selectedShaderChainer = shaderChainer;
  // When we select a ShaderChainer, we deselect the Shader
  selectedShader = nullptr;
}

void ShaderChainerService::selectShader(std::shared_ptr<Shader> shader) {
  selectedShader = shader;
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

      auto videoSources = VideoSourceService::getService()->videoSources();

      auto shaderChainer =
          new ShaderChainer(pair.first, chainerName, videoSources.at(0));
      shaderChainer->load(pair.second);
      shaderChainer->setup();
      addShaderChainer(std::shared_ptr<ShaderChainer>(shaderChainer));
    }
  }
}

std::shared_ptr<Shader>
ShaderChainerService::shaderForType(ShaderType type, std::string shaderId,
                                    json shaderJson) {

  switch (type) {
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
  }
}
