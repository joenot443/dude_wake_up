#include "Command.hpp"
#include "../VideoSourceService.hpp"

class AddLibraryVideoSourceCommand : public Command {
public:
  AddLibraryVideoSourceCommand(std::shared_ptr<LibraryFile> libraryFile, ImVec2 origin = ImVec2(0., 0.))
    : libraryFile(libraryFile), origin(origin) {}

  void execute() override {
    if (!videoSource) {
      videoSource = VideoSourceService::getService()->makeLibraryVideoSource(libraryFile);
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

  std::shared_ptr<LibraryFile> libraryFile;
  ImVec2 origin;
  std::shared_ptr<VideoSource> videoSource;
}; 
