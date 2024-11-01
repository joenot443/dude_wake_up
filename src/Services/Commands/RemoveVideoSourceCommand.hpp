#include "Command.hpp"
#include "../VideoSourceService.hpp"
#include "ShaderChainerService.hpp"
#include "Connection.hpp"

class RemoveVideoSourceCommand : public Command {
public:
  RemoveVideoSourceCommand(std::shared_ptr<VideoSource> videoSource)
    : videoSource(videoSource), videoSourceId(videoSource->id) {}

  void execute() override {
    connections = videoSource->connections();
    NodeLayoutView::getInstance()->closeSettingsWindow(videoSource);
    VideoSourceService::getService()->removeVideoSource(videoSourceId);
  }

  void undo() override {
    VideoSourceService::getService()->addVideoSource(videoSource, videoSourceId);
    for (const auto& connection : connections) {
      ShaderChainerService::getService()->makeConnection(
        connection->start, connection->end, connection->type,
        connection->outputSlot, connection->inputSlot);
    }
  }

  std::shared_ptr<VideoSource> getVideoSource() const {
    return videoSource;
  }

private:
  std::shared_ptr<VideoSource> videoSource;
  std::string videoSourceId;
  std::vector<std::shared_ptr<Connection>> connections;
}; 
