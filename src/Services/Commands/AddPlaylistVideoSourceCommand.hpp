#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddPlaylistVideoSourceCommand : public Command {
public:
  AddPlaylistVideoSourceCommand(const std::string& name, const std::string& path, ImVec2 origin = ImVec2(0., 0.))
    : name(name), path(path), origin(origin) {}

  void execute() override {
    if (!videoSource)
      videoSource = VideoSourceService::getService()->makeFileVideoSource(name, path);
    
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
  std::string path;
  ImVec2 origin;
  std::shared_ptr<VideoSource> videoSource;
}; 
