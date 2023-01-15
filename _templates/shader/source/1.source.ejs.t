---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: ShaderSource_<%= name %>
after: "enum ShaderSourceType {"
---
  ShaderSource_<%= name %>,