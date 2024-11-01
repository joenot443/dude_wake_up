#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddImageVideoSourceCommand : public Command {
public:
  AddImageVideoSourceCommand(const std::string& name, const std::string& path)
    : name(name), path(path) {}

  void execute() override {
    if (!videoSource) {
      videoSource = VideoSourceService::getService()->makeImageVideoSource(name, path);
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

private:
  std::string name;
  std::string path;
  std::shared_ptr<VideoSource> videoSource;
}; 
