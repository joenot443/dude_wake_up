---
to: src/Services/ShaderChainerService.cpp
inject: true
skip_if: case ShaderType<%= name %>
after: "switch"
---
    case ShaderType<%= name %>: {
      auto settings = new <%= name %>Settings(shaderId, shaderJson);
      auto shader = std::make_shared<<%= name %>Shader>(settings);
      shader->setup();
      return shader;
    }