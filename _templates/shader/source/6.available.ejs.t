---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: ShaderSource_<%= name %>, // Available
before: "}; // End AvailableShaderSourceTypes"
---
  ShaderSource_<%= name %>, // Available