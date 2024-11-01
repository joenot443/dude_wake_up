#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddWebcamVideoSourceCommand : public Command {
public:
  AddWebcamVideoSourceCommand(const std::string& name, int deviceId, ImVec2 origin = ImVec2(0., 0.))
    : name(name), deviceId(deviceId), origin(origin) {}

  void execute() override {
    if (!videoSource) {
      videoSource = VideoSourceService::getService()->makeWebcamVideoSource(name, deviceId);
    }
    VideoSourceService::getService()->addVideoSource(videoSource, videoSource->id, 0);
  }

  void undo() override {
    if (videoSource) {
      VideoSourceService::getService()->removeVideoSource(videoSource->id);
    }
  }

  std::shared_ptr<VideoSource> getVideoSource() const {
    return videoSource;
  }

  std::string name;
  int deviceId;
  ImVec2 origin;
  std::shared_ptr<VideoSource> videoSource;
}; 
