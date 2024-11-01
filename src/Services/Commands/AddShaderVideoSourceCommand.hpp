#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddShaderVideoSourceCommand : public Command {
public:
  AddShaderVideoSourceCommand(ShaderSourceType type, ImVec2 origin = ImVec2(0., 0.))
    : type(type), origin(origin) {}

  void execute() override {
    if (!videoSource) {
      videoSource = VideoSourceService::getService()->makeShaderVideoSource(type);
    }
    VideoSourceService::getService()->addVideoSource(videoSource, videoSource->id, 0);  }

  void undo() override {
    if (videoSource) {
      VideoSourceService::getService()->removeVideoSource(videoSource->id);
    }
  }

  std::shared_ptr<VideoSource> getVideoSource() const {
    return videoSource;
  }

  ShaderSourceType type;
  ImVec2 origin;
  std::shared_ptr<VideoSource> videoSource;
}; 
