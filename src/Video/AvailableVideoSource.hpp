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
  
  
  AvailableVideoSource(std::string sourceName, VideoSourceType type) :
  sourceName(sourceName),
  type(type),
  availableVideoSourceId(UUID::generateUUID()),
  shaderType(shaderType),
  webcamId(webcamId) {}
};
#endif /* AvailableVideoSource_h */
