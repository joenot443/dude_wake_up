//
//  ShaderSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef ShaderSource_hpp
#define ShaderSource_hpp

#include <stdio.h>
#include "VideoSource.hpp"
#include "UUID.hpp"
#include "VideoSettings.hpp"
#include "FujiShader.hpp"
#include "FractalShader.hpp"
#include "PlasmaShader.hpp"
#include "Shader.hpp"

enum ShaderSourceType { ShaderSource_plasma, ShaderSource_fractal, ShaderSource_fuji };

static std::string shaderSourceTypeName(ShaderSourceType type) {
  switch (type) {
    case ShaderSource_plasma:
      return "Plasma";
    case ShaderSource_fractal:
      return "Fractal";
    case ShaderSource_fuji:
      return "Fuji";
    default:
      return "Unknown";
  }
};

struct ShaderSource : public VideoSource {
public:
  std::shared_ptr<Shader> shader;
  ShaderSourceType shaderType;
  ofFbo fbo;
  ofFbo canvas;
  
  ShaderSource(std::string id, ShaderSourceType type) :
  shader(nullptr),
  VideoSource(id, shaderSourceTypeName(type), VideoSource_chainer)
  {
    addShader(type);
  };
  
  void addShader(ShaderSourceType type) {
    switch (type) {
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
//    canvas.getTexture().copyTo(frameBuffer);
//
//    frameTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);
  }
  
  void drawSettings() override {
    if (shader != nullptr) {
      shader->drawSettings();
    }
  }
  
  void update();
  void draw();
  
};

#endif /* ShaderSource_hpp */
