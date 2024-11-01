#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddTextVideoSourceCommand : public Command {
public:
  AddTextVideoSourceCommand(const std::string& name, ImVec2 origin = ImVec2(0., 0.))
    : name(name), origin(origin) {}

  void execute() override {
    if (!videoSource) {
      videoSource = VideoSourceService::getService()->makeTextVideoSource(name);
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

  std::string name;
  ImVec2 origin;
  std::shared_ptr<VideoSource> videoSource;
}; 
