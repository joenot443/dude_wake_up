---
to: src/Video/ShaderSource.hpp
inject: true
skip_if: <%= name %>Settings
after: "    // Shader Settings"
---
    case ShaderSource_<%= name %>: { // Settings
      auto settings = new <%= name %>Settings(UUID::generateUUID(), 0);
      shader = std::make_shared<<%= name %>Shader>(settings);
      shader->setup();
      return;
    }