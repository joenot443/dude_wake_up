---
to: src/Shading/ShaderType.hpp
inject: true
after: "// Available shaders"
skip_if: "ShaderType<%= name %>, // Generated"
---
  ShaderType<%= name %>, // Generated