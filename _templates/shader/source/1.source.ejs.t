---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: "ShaderSource_<%= name %>, //source enum"
after: "enum ShaderSourceType {"
---
  ShaderSource_<%= name %>, //source enum,