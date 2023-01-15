---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: #include "<%= name %>Shader.hpp"
after: "#include"
---
#include "<%= name %>Shader.hpp"