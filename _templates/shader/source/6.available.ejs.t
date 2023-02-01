---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: <%= name %>Settings
after: "  // Available ShaderSourceTypes"
---
    ShaderSource_<%= name %>,