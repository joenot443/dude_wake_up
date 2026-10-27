//
//  CachedShader.hpp
//  dude_wake_up
//
//  An ofShader whose load(name) shares one compiled program among every
//  instance loading the same files (ofShader copies share their program,
//  reference counted), so ten Kaleidoscope nodes compile once.
//

#ifndef CachedShader_hpp
#define CachedShader_hpp

#include "ofMain.h"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

class CachedShader : public ofShader {
public:
  using ofShader::ofShader;
  using ofShader::operator=;  // e.g. an edited program compiled into a fresh ofShader
  using ofShader::load;

  // Loads `name`.vert/.frag, reusing the program already compiled from
  // them unless a file has changed since (an edited effect file). Engine
  // (GL) thread only.
  bool load(const of::filesystem::path& name) {
    const std::string key = name.string();
    const auto stamp = fileStamp(key);
    auto& cache = programs();
    auto it = cache.find(key);
    samplerProgram_ = 0;
    if (it != cache.end() && it->second.stamp == stamp && it->second.shader.isLoaded()) {
      ofShader::operator=(it->second.shader);
      return true;
    }
    // ofShader::load compiles into the current program; never one shared
    // with other instances.
    unload();
    const bool ok = ofShader::load(name);
    if (ok) cache[key] = {static_cast<const ofShader&>(*this), stamp};
    return ok;
  }

  // A program shared by several nodes keeps the uniforms the last of them
  // set. Values set every frame don't care, but a sampler only set sometimes
  // (an optional second input) would read another node's texture unit:
  // start each use with every sampler on unit 0, as a fresh program has.
  void begin() const {
    ofShader::begin();
    const GLuint program = getProgram();
    if (program == 0) return;
    if (program != samplerProgram_) {
      samplerProgram_ = program;
      samplers_ = samplerLocations(program);
    }
    for (GLint location : samplers_) glUniform1i(location, 0);
  }

private:
  struct Entry {
    ofShader shader;
    std::vector<long long> stamp;
  };

  // Never destroyed: by exit the GL context is gone.
  static std::map<std::string, Entry>& programs() {
    static auto* cache = new std::map<std::string, Entry>();
    return *cache;
  }

  static std::vector<long long> fileStamp(const std::string& name) {
    std::vector<long long> stamp;
    for (const char* ext : {".vert", ".frag"}) {
      std::error_code ec;
      const auto time = std::filesystem::last_write_time(ofToDataPath(name + ext, true), ec);
      stamp.push_back(ec ? 0 : static_cast<long long>(time.time_since_epoch().count()));
    }
    return stamp;
  }

  static std::vector<GLint> samplerLocations(GLuint program) {
    std::vector<GLint> locations;
    GLint count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    for (GLint i = 0; i < count; i++) {
      char uniformName[256];
      GLsizei length = 0;
      GLint size = 0;
      GLenum type = 0;
      glGetActiveUniform(program, GLuint(i), sizeof(uniformName), &length, &size, &type, uniformName);
      switch (type) {
        case GL_SAMPLER_1D: case GL_SAMPLER_2D: case GL_SAMPLER_3D: case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D_RECT: case GL_SAMPLER_2D_ARRAY: case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D: case GL_UNSIGNED_INT_SAMPLER_2D: {
          const GLint location = glGetUniformLocation(program, uniformName);
          if (location >= 0) locations.push_back(location);
          break;
        }
        default:
          break;
      }
    }
    return locations;
  }

  mutable GLuint samplerProgram_ = 0;
  mutable std::vector<GLint> samplers_;
};

#endif
