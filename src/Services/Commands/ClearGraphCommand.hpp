#ifndef ClearGraphCommand_hpp
#define ClearGraphCommand_hpp

#include "Command.hpp"
#include "ShaderChainerService.hpp"
#include "../VideoSourceService.hpp"
#include "Connection.hpp"
#include "ConfigService.hpp"

class ClearGraphCommand : public Command {
public:
  ClearGraphCommand() {}

  void execute() override {
    // Capture all state before clearing
    savedShaders = ShaderChainerService::getService()->shaders();
    savedSources = VideoSourceService::getService()->videoSources();
    savedConnections = ShaderChainerService::getService()->connections();

    // Clear the graph
    VideoSourceService::getService()->clear();
    ShaderChainerService::getService()->clear();
    ConfigService::getService()->closeWorkspace();
  }

  void undo() override {
    // Re-add all video sources
    for (auto const& source : savedSources) {
      VideoSourceService::getService()->addVideoSource(source, source->id);
    }

    // Re-add all shaders
    for (auto const& shader : savedShaders) {
      ShaderChainerService::getService()->addShader(shader);
    }

    // Re-add all connections
    for (auto const& connection : savedConnections) {
      ShaderChainerService::getService()->makeConnection(
        connection->start, connection->end, connection->type,
        connection->outputSlot, connection->inputSlot);
    }
  }

private:
  std::vector<std::shared_ptr<Shader>> savedShaders;
  std::vector<std::shared_ptr<VideoSource>> savedSources;
  std::vector<std::shared_ptr<Connection>> savedConnections;
};

#endif
