//
//  EngineConfig.hpp
//  dude_wake_up
//
//  Configuration and preprocessor definitions for the engine library.
//
//  When building as a static library for the SwiftUI frontend,
//  define NOTTAWA_ENGINE_ONLY=1 to exclude UI-specific code paths.
//
//  NOTE: ImGui headers remain in the build since engine types reference
//  ImVec2, ImColor, etc. in their declarations. The engine never *calls*
//  ImGui draw functions during tick() -- those are only invoked by the
//  ImGui frontend's draw loop. This means ImGui types compile fine as
//  data-only members without a live ImGui context.
//

#ifndef EngineConfig_hpp
#define EngineConfig_hpp

// When defined, UI-specific code (draw calls, ImGui rendering) is excluded.
// This is set in the static library target's preprocessor macros.
// #define NOTTAWA_ENGINE_ONLY 1

#endif /* EngineConfig_hpp */
