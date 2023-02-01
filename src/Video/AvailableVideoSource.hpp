//
//  AvailableVideoSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 1/10/23.
//

#ifndef AvailableVideoSource_h
#define AvailableVideoSource_h
#include "VideoSource.hpp"
#include "UUID.hpp"
#include "ShaderSource.hpp"

struct AvailableVideoSource {
  std::string availableVideoSourceId;
  std::string sourceName;
  VideoSourceType type;
  ShaderSourceType shaderType;
  int webcamId;
  std::string path;
  std::shared_ptr<ofTexture> preview;
  
  void generatePreview();
  
  AvailableVideoSource(std::string sourceName, VideoSourceType type, ShaderSourceType shaderType, int webcamId, std::string path);
};
#endif /* AvailableVideoSource_h */
