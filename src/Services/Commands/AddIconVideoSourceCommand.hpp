#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddIconVideoSourceCommand : public Command {
public:
  AddIconVideoSourceCommand(const std::string& name)
    : name(name) {}

  void execute() override {
    if (!videoSource)
    	videoSource = VideoSourceService::getService()->makeIconVideoSource(name);
    
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
  std::shared_ptr<VideoSource> videoSource;
}; 
