//
//  Settings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef VideoSettings_h
#define VideoSettings_h

#include "Oscillator.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include "Strings.hpp"
#include "ShaderSettings.hpp"
#include <string>
#include "json.hpp"
#include "macro_scope.hpp"

using json = nlohmann::json;


struct VideoFlags {
  Parameter resetFeedback;
  std::string shaderId;
  VideoFlags(std::string shaderId) : resetFeedback(Parameter("reset_feedback", shaderId, 0.0, 0.0, 1.0)), shaderId(shaderId) {}
};

struct VideoSettings {
  VideoFlags videoFlags;
  int streamId;
  std::string shaderIdStr;
  
  VideoSettings(int intId, std::string strId) :
  streamId(intId),
  shaderIdStr(strId),
  videoFlags(strId)
  {
  }
};



#endif /* Settings_h */
