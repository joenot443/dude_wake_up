---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: "ShaderSource_<%= name %>, //source enum"
before: "}; // End ShaderSourceType"
---
  ShaderSource_<%= name %>, //source enum,