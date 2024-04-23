---
to: src/Shading/ShaderType.hpp
inject: true
skip_if: "case ShaderType<%= name %>"
after: "ShaderNames"
---
    case ShaderType<%= name %>:
      return "<%= name %>";