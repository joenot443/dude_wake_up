#include "Command.hpp"
#include "../ShaderChainerService.hpp"

class AddShaderCommand : public Command {
public:
  AddShaderCommand(std::shared_ptr<Shader> shader) : shader(shader) {}

  void execute() override {
    ShaderChainerService::getService()->addShader(shader);
  }

  void undo() override {
    ShaderChainerService::getService()->removeShader(shader);
  }

private:
  std::shared_ptr<Shader> shader;
};
