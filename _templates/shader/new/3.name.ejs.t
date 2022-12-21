---
to: src/ShaderType.hpp
inject: true
skip_if: "case ShaderType<%= name %>"
after: "// Shader names"
---
    case ShaderType<%= name %>:
      return "<%= name %>";