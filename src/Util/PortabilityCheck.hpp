#pragma once

// Compile-time canary: fires if the Windows build is missing NOMINMAX.
//
// Why: <Windows.h> defines `min` and `max` as preprocessor macros unless
// NOMINMAX is defined *before* Windows.h is first included. Those macros
// collide with:
//   - Parameter::min / Parameter::max          (member variables)
//   - WaveformOscillator::min() / max()        (member functions)
//   - std::min / std::max
// causing cryptic template errors deep in the shader pipeline. NOMINMAX
// must be a project-wide preprocessor define in the Windows build config.
//
// This header does the check cheaply (no Windows.h include). The paired
// canary TU src/Util/PortabilityCheck.cpp forces the check to run in a
// context where <Windows.h> has already been included, so the compiler
// error surfaces on every misconfigured build.

#ifdef _WIN32
#  ifdef min
#    error "NOMINMAX must be defined project-wide before any <Windows.h> include. See src/Util/PortabilityCheck.hpp — this exists to make the failure loud instead of cryptic in Parameter/WaveformOscillator."
#  endif
#  ifdef max
#    error "NOMINMAX must be defined project-wide before any <Windows.h> include. See src/Util/PortabilityCheck.hpp — this exists to make the failure loud instead of cryptic in Parameter/WaveformOscillator."
#  endif
#endif
