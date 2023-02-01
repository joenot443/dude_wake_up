---
to: src/Shading/ShaderType.hpp
inject: true
before: "// Available shaders"
skip_if: "ShaderType<%= name %>, // Generated"
---
  ShaderType<%= name %>, // Generated