---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: "case ShaderSource_<%= name %>: //type enum"
after: "// shaderTypeForShaderSourceType"
---
  case ShaderSource_<%= name %>: //type enum
    return ShaderType<%= name %>;