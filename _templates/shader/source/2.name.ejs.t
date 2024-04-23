---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: return "<%= name %>"; // <%= name %>
after: "ShaderNames"
---
  case ShaderSource_<%= name %>: // Name  
    return "<%= name %>"; // <%= name %>