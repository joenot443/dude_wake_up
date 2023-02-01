---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: <%= name %>Settings
after: "  // shaderTypeForShaderSourceType"
---
    case ShaderSource_<%= name %>: // Settings
      return ShaderType<%= name %>;