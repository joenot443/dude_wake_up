#include "Command.hpp"
#include "../ShaderChainerService.hpp"

class RemoveShaderCommand : public Command {
public:
  RemoveShaderCommand(std::shared_ptr<Shader> shader) : shader(shader) {}

  void execute() override {
    ShaderChainerService::getService()->removeShader(shader);
  }

  void undo() override {
    ShaderChainerService::getService()->addShader(shader);
  }

private:
  std::shared_ptr<Shader> shader;
};
