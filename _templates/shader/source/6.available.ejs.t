---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: ShaderSource_<%= name %>, // Available
after: "// Available ShaderSourceTypes"
---
  ShaderSource_<%= name %>, // Available