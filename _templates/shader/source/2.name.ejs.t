---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: return "<%= name %>"; // <%= name %>
after: "  // Shader Names"
---
  case ShaderSource_<%= name %>: // Name  
    return "<%= name %>"; // <%= name %>