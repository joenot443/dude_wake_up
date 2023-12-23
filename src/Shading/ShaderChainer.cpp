////
////  ShaderChainer.cpp
////  dude_wake_up
////
////  Created by Joe Crozier on 8/30/22.
////
//
//#include "ShaderChainer.hpp"
//#include "ASCIIShader.hpp"
//#include "BlurShader.hpp"
//#include "ConfigService.hpp"
//#include "Console.hpp"
//#include "FeedbackShader.hpp"
//#include "FeedbackSourceService.hpp"
//#include "GlitchShader.hpp"
//#include "HSBShader.hpp"
//#include "KaleidoscopeShader.hpp"
//#include "MirrorShader.hpp"
//#include "MixShader.hpp"
//#include "PixelShader.hpp"
//#include "ShaderChainerService.hpp"
//#include "TileShader.hpp"
//#include "TransformShader.hpp"
//#include "VideoSourceService.hpp"
//#include "ofMain.h"
//
//static const int EmptyId = -1;
//
//void ShaderChainer::setup()
//{
//  registerFeedbackDestination();
//  fbo = ofFbo();
//  fbo.allocate(settings.width->value, settings.height->value, GL_RGBA);
//  fbo.begin();
//  ofSetColor(0, 0, 0, 0);
//  ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
//  fbo.end();
//  fbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
//
//  resizeShader.load("shaders/Resize");
//  for (auto &sh : shaders())
//  {
//    sh->setup();
//  }
//}
//
//void ShaderChainer::update()
//{
//  // If the width of our Source or the width of the Chainer itself (the output)
//  // has changed, then setup again to recreate the textures.
//  if (fbo.getWidth() != settings.width->value || source->settings.width->value != frameTexture->getWidth())
//  {
//    setup();
//  }
//  processFrame(source->frameTexture);
//  resizeFrame();
//  saveFeedbackFrame();
//}
//
//// Recursively iterates through the std::set shader->next to collect every Shader in the chain.
//std::vector<std::shared_ptr<Shader>> collectShaders(std::shared_ptr<Shader> shader, std::vector<std::shared_ptr<Shader>> *shaders = {})
//{
//  if (!shader) return *shaders;  // Add null pointer check
//
//  shaders->push_back(shader);
//  if (shader->next.size() > 0)
//  {
//    for (auto next : shader->next)
//    {
//      collectShaders(next, shaders);
//    }
//  }
//  return *shaders;
//}
//
//std::vector<std::shared_ptr<Shader>> ShaderChainer::shaders()
//{
//  std::vector<std::shared_ptr<Shader>> shaders;
//  collectShaders(front, &shaders);
//  return shaders;
//}
//
///// Draws our `frameTexture` into our `fbo` at the size specified by our `ShaderChainer`.
//void ShaderChainer::resizeFrame()
//{
//  fbo.begin();
//  resizeShader.begin();
//  resizeShader.setUniform2f("inDimensions", frameTexture->getWidth(), frameTexture->getHeight());
//  resizeShader.setUniform2f("outDimensions", fbo.getWidth(), fbo.getHeight());
//  resizeShader.setUniformTexture("tex", *frameTexture.get(), 4);
//  ofClear(0, 0);
//  ofSetColor(0, 0, 0, 0);
//  ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
//  fbo.draw(0, 0);
//  resizeShader.end();
//  fbo.end();
//}
//
///*
//
// */
//void ShaderChainer::processFrame(std::shared_ptr<ofTexture> texture)
//{
//  fbo.begin();
////  ofClear(0, 0);
////  ofSetColor(0, 0, 0, 0);
////  ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
////  ofSetColor(0, 0, 0, 255);
//  texture->draw(0, 0);
//  fbo.end();
//
//  // Return raw texture if no shaders
//  if (front == nullptr)
//  {
//    return;
//  }
//
//  front->traverseFrame(&fbo);
//}
//
//void ShaderChainer::saveFeedbackFrame()
//{
//  if (feedbackDestination)
//  {
//    feedbackDestination->pushFrame(fbo);
//  }
//}
//
//json ShaderChainer::serialize()
//{
//  json j;
//  j["shaders"] = json::array();
//  j["chainerId"] = chainerId;
//  j["name"] = name;
//  j["sourceId"] = source->id;
//
//  // We have a front shader
//  if (front != nullptr)
//  {
//    j["front"] = front->shaderId;
//  }
//  else
//  {
//    j["front"] = EmptyId;
//  }
//
//  if (frontAux != nullptr)
//  {
//    j["frontAux"] = frontAux->shaderId;
//  }
//  else
//  {
//    j["frontAux"] = EmptyId;
//  }
//
//  for (auto shader : shaders())
//  {
//    j["shaders"].push_back(shader->serialize());
//  }
//  return j;
//}
//
//void ShaderChainer::load(json j)
//{
//  //  std::cout << j.dump(4) << std::endl;
//
//  if (VideoSourceService::getService()->videoSourceForId(j["sourceId"]) !=
//      nullptr)
//  {
//    source = VideoSourceService::getService()->videoSourceForId(j["sourceId"]);
//  }
//
//  chainerId = j["chainerId"];
//
//  std::shared_ptr<Shader> last = nullptr;
//
//  for (auto shaderJson : j["shaders"])
//  {
//    ShaderType shaderType = shaderJson["shaderType"];
//    std::string shaderId = shaderJson["shaderId"];
//
//    auto shader = ShaderChainerService::getService()->shaderForType(
//        shaderType, shaderId, shaderJson);
//    ShaderChainerService::getService()->addShader(shader);
//    ShaderChainerService::getService()->associateShaderWithChainer(shaderId, chainerId);
//  }
//
//  for (auto shaderJson : j["shaders"])
//  {
//    auto shader = ShaderChainerService::getService()->shaderForId(shaderJson["shaderId"]);
//    shader->parent = ShaderChainerService::getService()->shaderForId(shaderJson["parent"]);
//    // Populate the std::set shader->next with the shaders from shaderJson["next"]
//    if (shaderJson["next"].is_array())
//    {
//      for (auto nextId : shaderJson["next"])
//      {
//        shader->next.insert(ShaderChainerService::getService()->shaderForId(nextId));
//      }
//    }
//  }
//
//  if (j["front"] != EmptyId)
//  {
//    front = ShaderChainerService::getService()->shaderForId(id);
//  }
//
//  if (j["frontAux"] != EmptyId)
//  {
//    frontAux = ShaderChainerService::getService()->shaderForId(j["frontAux"]);
//  }
//}
//
//void ShaderChainer::registerFeedbackDestination()
//{
//  std::shared_ptr<FeedbackSource> source =
//      std::make_shared<FeedbackSource>(chainerId, sourceName, std::make_shared<VideoSourceSettings>(settings));
//  FeedbackSourceService::getService()->registerFeedbackSource(source);
//  feedbackDestination = source;
//}
