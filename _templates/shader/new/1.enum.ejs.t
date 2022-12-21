---
to: src/ShaderType.hpp
inject: true
skip_if: "ShaderType<%= name %>,"
before: "};"
---
  ShaderType<%= name %>,