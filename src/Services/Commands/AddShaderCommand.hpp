#include "Command.hpp"
#include "ShaderChainerService.hpp"
#include "ShaderType.hpp"

class AddShaderCommand : public Command {
public:
  AddShaderCommand(ShaderType shaderType) : shaderType(shaderType) {}

  void execute() override {
    if (!shader) {
      shader = ShaderChainerService::getService()->makeShader(shaderType);
    } else {
      // On redo, re-add the shader that was removed by undo
      ShaderChainerService::getService()->addShader(shader);
    }
  }

  void undo() override {
    if (shader) {
      ShaderChainerService::getService()->removeShader(shader);
    }
  }

  std::shared_ptr<Shader> getShader() const {
    return shader;
  }

  ShaderType shaderType;
  std::shared_ptr<Shader> shader;
};
