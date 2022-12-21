---
to: src/Services/ShaderChainerService.cpp
inject: true
skip_if: "#include \"<%= name %>Shader.hpp\""
after: "#include \"AsciiShader.hpp\""
---
#include "<%= name %>Shader.hpp"