//
//  ShaderSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef ShaderSource_hpp
#define ShaderSource_hpp

#include <stdio.h>
#include "EmptyShader.hpp"
#include "VideoSource.hpp"
#include "UUID.hpp"
#include "CloudShader.hpp"
#include "VideoSettings.hpp"
#include "FujiShader.hpp"
#include "MelterShader.hpp"
#include "FractalShader.hpp"
#include "PlasmaShader.hpp"
#include "Shader.hpp"

using json = nlohmann::json;

enum ShaderSourceType { ShaderSource_empty, ShaderSource_plasma, ShaderSource_fractal, ShaderSource_fuji, ShaderSource_clouds, ShaderSource_melter };

static std::string shaderSourceTypeName(ShaderSourceType type) {
  switch (type) {
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
  
  ShaderSource(std::string id, ShaderSourceType type) :
  shader(nullptr),
  shaderSourceType(type),
  VideoSource(id, shaderSourceTypeName(type), VideoSource_shader)
  {
    addShader(type);
  };
  
  void addShader(ShaderSourceType type) {
    switch (type) {
      case ShaderSource_empty: {
        auto plasmaSettings = new EmptySettings(UUID::generateUUID(), 0);
        shader = std::make_shared<EmptyShader>(plasmaSettings);
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
    frameBuffer.allocate(1280*720*4, GL_STATIC_COPY);
    
    shader->setup();
    fbo.allocate(1280, 720, GL_RGB);
    fbo.begin();
    ofClear(0,0,0,255);
    fbo.end();
    
    canvas.allocate(1280, 720, GL_RGB);
    canvas.begin();
    ofClear(0,0,0,255);
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
