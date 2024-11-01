#include "Command.hpp"
#include "ShaderChainerService.hpp"
#include "NodeLayoutView.hpp"
#include "Connection.hpp"

class RemoveShaderCommand : public Command {
public:
  RemoveShaderCommand(std::shared_ptr<Shader> shader)
    : shader(shader) {}

  void execute() override {
    // Save connections before removing the shader
    connections = shader->connections();
    NodeLayoutView::getInstance()->closeSettingsWindow(shader);
    ShaderChainerService::getService()->removeShader(shader);
  }

  void undo() override {
    // Re-add the shader
    ShaderChainerService::getService()->addShader(shader);
    // Re-add the saved connections
    for (const auto& connection : connections) {
      ShaderChainerService::getService()->makeConnection(
        connection->start, connection->end, connection->type,
        connection->outputSlot, connection->inputSlot);
    }
  }

  std::shared_ptr<Shader> getShader() const {
    return shader;
  }

private:
  std::shared_ptr<Shader> shader;
  std::vector<std::shared_ptr<Connection>> connections;
};
