---
to: src/Shading/ShaderType.hpp
inject: true
skip_if: "ShaderType<%= name %>,"
before: "};"
---
  ShaderType<%= name %>,