//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include <regex>
#include "Shader.hpp"
#include "NodeLayoutView.hpp"
#include "Colors.hpp"
#include "LayoutStateService.hpp"
#include "Console.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "FeedbackSourceService.hpp"
#include "LayoutStateService.hpp"


void Shader::traverseFrame(std::shared_ptr<ofFbo> frame, int depth)
{
  if (!active) return;
  if (!frame || !frame->isAllocated()) return;
  if (!lastFrame || !lastFrame->isAllocated()) return;
  if (frame->getTexture().getTextureData().textureID == 0) return;
  if (lastFrame->getTexture().getTextureData().textureID == 0) return;

  clearLastFrame();
  checkForFileChanges();
  activateParameters();

  // Bypass: copy input to output without processing
  if (bypassed) {
    lastFrame->begin();
    frame->draw(0, 0);
    lastFrame->end();
  } else if (!isDry || allowsDryTraversal()) {
    shade(frame, lastFrame);
  }

  applyOptionalShaders();
  
  // On our terminal nodes, check if we need to update our defaultStageShader
  if (outputs.empty()) {
    if (depth > ParameterService::getService()->defaultStageShaderIdDepth.second) {
      ParameterService::getService()->defaultStageShaderIdDepth = std::pair<std::string, int>(shaderId, depth);
    }
  }
  
  for (auto [slot, connections] : outputs) // Iterate over every connection
  {
    for (auto &connection : connections) {
      if (connection->inputSlot != InputSlotMain) { continue; }
      if (connection->outputSlot != OutputSlotMain) { continue; }
      
      std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
      if (shader)
      {
        shader->isDry = isDry;
        shader->traverseFrame(lastFrame, depth + 1);
      }
    }
  }

  // If necessary, copy the texture to the Shader's FeedbackDestination
  if (feedbackDestination != nullptr && feedbackDestination->beingConsumed() && feedbackDestination->type == FeedbackType_full)
  {
    feedbackDestination->pushFrame(lastFrame);
  }

  // Mark the FBO as freshly written so IOSurfaceTextureManager knows to blit.
  ++renderTick;
}

void Shader::clearLastFrame() {
  lastFrame->begin();
  ofSetColor(0, 0, 0, 255);
  ofDrawRectangle(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
  ofClear(0,0,0, 255);
  ofClear(0,0,0, 0);
  lastFrame->end();
  ++renderTick;
}

std::shared_ptr<VideoSourceSettings> Shader::sourceSettings() {
  if (hasParentOfType(ConnectableTypeSource)) {
    std::shared_ptr<Connectable> parentSource = parentOfType(ConnectableTypeSource);
    
    std::shared_ptr<VideoSource> castedSource = std::dynamic_pointer_cast<VideoSource>(parentSource);
    return castedSource->settings;
  } else {
    return VideoSourceService::getService()->defaultVideoSourceSettings();
  }
}

void Shader::load(json j)
{
  if (!j.is_object()) return;

  // Restore optional shaders (enabled state + parameters)
  if (j.contains("optionalShaders") && !isOptional) optionalShaders.load(j["optionalShaders"]);
}

json Shader::serialize()
{
  json j = settings->serialize();
  j["shaderType"] = type();
  j["shaderId"] = settings->shaderId;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId);
  if (node != nullptr)
  {
    settings->x->value = node->position.x;
    settings->y->value = node->position.y;
  }
  // Always serialize x/y from settings (bridge sets these in NottawaApp).
  j["x"] = settings->x->value;
  j["y"] = settings->y->value;

  // Serialize optional shaders (enabled state + parameters)
  json optJson = optionalShaders.serialize();
  if (!optJson.is_null()) j["optionalShaders"] = optJson;

  return j;
};

void Shader::activateParameters() {
  auto* paramService = ParameterService::getService();
  for (std::shared_ptr<Parameter> param : settings->parameters) {
    paramService->markActive(param);
  }
}

void Shader::allocateFrames() {
  lastFrame->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  // optionalFrame is allocated when an optional shader first runs.
}

void Shader::releaseFrames() {
  if (lastFrame) lastFrame->clear();
  if (optionalFrame) optionalFrame->clear();
  for (auto& opt : optionalShaders.created()) opt->releaseFrames();
}

void Shader::enableAudioAutoReactivity(std::shared_ptr<Parameter> audioParam) {
  if (settings->audioReactiveParameter == nullptr) return;
  
  settings->audioReactiveParameter->addDriver(audioParam);
}

void Shader::disableAudioAutoReactivity() {
  settings->audioReactiveParameter->removeDriver();
}

std::shared_ptr<ofFbo> Shader::parentFrame() {
    for (auto const& [key, connection] : inputs) {
      return connection->start->frame();
    }
    return nullptr;
}

void Shader::checkForFileChanges() {
  if (!AllowShaderMonitoring) return;
  
  std::string shaderName = fileName();
  auto path = "shaders/" + shaderName + ".frag";
  
  auto shaderFile = ofFile(path);
  
  if (!shaderFile.exists()) {
//    log("Shader doesn't exist - " + name());
    return;
  }
  
  // Check when the file was last modified
  auto time = std::filesystem::last_write_time(shaderFile);
  auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
  
  if (timeSinceEpoch > lastModified) {
    lastModified = timeSinceEpoch;
    shader.unload();
    // Print the contents of the shader file
    log("Shader file contents - " + shaderFile.readToBuffer().getText());
    
    shader.load("shaders/" + shaderName);
    log("Reloaded Shader for - " + shaderName);
  }
}

// ---------------------------------------------------------------------------
// MARK: - Live GLSL editing
// ---------------------------------------------------------------------------

namespace {

// Standard Nottawa vertex passthrough. Matches the .vert files shipped
// alongside every fragment shader. Used as a fallback when a .vert file
// is missing on disk (a few shaders ship frag-only).
constexpr const char* kDefaultPassthroughVert = R"GLSL(#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;

out vec2 coord;

void main()
{
  coord = vec2(texcoord.x, texcoord.y);
  gl_Position = modelViewProjectionMatrix * position;
}
)GLSL";

// RAII guard that swaps in a buffering log channel for the duration of a
// scoped block, then restores whatever channel was active before. Used to
// capture ofShader compile / link errors (which arrive via ofLogError,
// since ofShader exposes no info-log return).
//
// Note: ofLog's channel is process-global, so any unrelated log call on the
// engine thread during the guarded window will also be captured. Keep the
// window as tight as possible — wrap only the compile + link calls.
class ScopedShaderLogCapture {
public:
  ScopedShaderLogCapture() {
    saved_ = ofGetLoggerChannel();
    buffer_ = std::make_shared<BufferingChannel>();
    ofSetLoggerChannel(buffer_);
  }
  ~ScopedShaderLogCapture() {
    ofSetLoggerChannel(saved_);
  }
  ScopedShaderLogCapture(const ScopedShaderLogCapture&) = delete;
  ScopedShaderLogCapture& operator=(const ScopedShaderLogCapture&) = delete;

  std::string text() const { return buffer_->text; }

private:
  class BufferingChannel : public ofBaseLoggerChannel {
  public:
    std::string text;
    // Only the std::string overload is virtual on the base; the variadic
    // template forms route through it via ofVAArgsToString.
    void log(ofLogLevel /*level*/, const std::string& module, const std::string& message) override {
      // Anything from ofShader (or its prefix variants) is interesting.
      if (module.find("ofShader") != std::string::npos) {
        text += message;
        if (!text.empty() && text.back() != '\n') text += '\n';
      }
    }
  };

  std::shared_ptr<ofBaseLoggerChannel> saved_;
  std::shared_ptr<BufferingChannel> buffer_;
};

// Extract the first GLSL error line number from a compiler log. GLSL drivers
// produce messages like "0:42(7): error:" (Intel) or "ERROR: 0:42:" (NVIDIA);
// the second number is the source line. Returns 0 if no line found.
int parseFirstErrorLine(const std::string& log) {
  static const std::regex re(R"((?:0:|ERROR:\s*\d+:)\s*(\d+))");
  std::smatch match;
  if (std::regex_search(log, match, re) && match.size() >= 2) {
    try { return std::stoi(match[1].str()); } catch (...) {}
  }
  return 0;
}

// Read a file's full contents into a string. Returns empty on miss.
std::string readShaderFile(const std::string& path) {
  ofFile f(path);
  if (!f.exists()) return {};
  return ofBufferFromFile(path).getText();
}

} // namespace

std::string Shader::currentFragmentSource() {
  if (hasSessionOverride()) return editedFragmentSource;
  return readShaderFile("shaders/" + fileName() + ".frag");
}

bool Shader::applyFragmentSourceOverride(const std::string& source) {
  lastCompileError.clear();
  lastCompileErrorLine = 0;

  // Resolve the vertex source: prefer the shader's own .vert if present,
  // otherwise fall back to the standard passthrough.
  std::string vertSource = readShaderFile("shaders/" + fileName() + ".vert");
  if (vertSource.empty()) vertSource = kDefaultPassthroughVert;

  // Stage a fresh ofShader. If anything fails, we throw it away without
  // touching the live `shader` member — the previously-valid program keeps
  // rendering.
  ofShader staged;
  bool ok = true;
  std::string log;
  {
    ScopedShaderLogCapture capture;
    ok = staged.setupShaderFromSource(GL_VERTEX_SHADER, vertSource);
    if (ok) ok = staged.setupShaderFromSource(GL_FRAGMENT_SHADER, source);
    if (ok) {
      staged.bindDefaults();
      ok = staged.linkProgram();
    }
    log = capture.text();
  }

  if (!ok) {
    lastCompileError = log.empty() ? "Shader compile failed (no detail)." : log;
    lastCompileErrorLine = parseFirstErrorLine(log);
    return false;
  }

  // Atomic-from-the-engine-thread-POV swap. shade() can't run concurrently
  // because this method only runs inside runOnEngineThread, and the engine
  // thread is serialized.
  shader = std::move(staged);
  editedFragmentSource = source;
  return true;
}

void Shader::revertFragmentSource() {
  editedFragmentSource.clear();
  lastCompileError.clear();
  lastCompileErrorLine = 0;
  shader.unload();
  shader.load("shaders/" + fileName());
}

void Shader::drawOptionalSettings() {
  optionalShaders.drawOptionalSettings();
}

void Shader::applyOptionalShaders() {
  if (isOptional) return;
  optionalShaders.applyOptionalShaders(lastFrame, optionalFrame);
}

void Shader::drawPreview(ImVec2 pos, float scale)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImGui::Image(texID, LayoutStateService::getService()->previewSize(scale));
}

void Shader::drawPreviewSized(ImVec2 size)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImVec2 cursorPos = ImGui::GetCursorPos();

  float fboW = lastFrame->getWidth();
  float fboH = lastFrame->getHeight();
  if (fboW <= 0 || fboH <= 0) return;

  float fboAspect = fboW / fboH;
  float boxAspect = size.x / size.y;

  ImVec2 drawSize;
  if (fboAspect > boxAspect) {
    drawSize.x = size.x;
    drawSize.y = size.x / fboAspect;
  } else {
    drawSize.y = size.y;
    drawSize.x = size.y * fboAspect;
  }

  ImVec2 offset((size.x - drawSize.x) * 0.5f, (size.y - drawSize.y) * 0.5f);
  ImVec2 topLeft = cursorPos + offset;

  ImGui::GetWindowDrawList()->AddImageRounded(
    texID,
    topLeft,
    topLeft + drawSize,
    ImVec2(0,0),
    ImVec2(1,1),
    IM_COL32_WHITE,
    8.0
  );
}



bool Shader::allowAuxOutputSlot() {
  return type() == ShaderTypeFeedback || type() == ShaderTypeColorKeyMaskMaker || type() == ShaderTypeLumaMaskMaker;
}

void Shader::breakAuxillary() {
  if (hasOutputAtSlot(OutputSlotAux)) {
    // Get all connections on the auxiliary output slot
    auto connections = outputs[OutputSlotAux];

    // Break each connection
    for (auto connection : connections) {
      ShaderChainerService::getService()->breakConnectionForConnectionId(connection->id);
    }
  }
}

void Shader::setAudioUniform(std::vector<float> *audio) {
  // Make a copy to avoid threading issues
  std::vector<float> audioCopy = *audio;
  if (audioCopy.size() == 256) {
    shader.setUniform1fv("audio", &audioCopy[0], 256);
  }
}

void Shader::setAudioUniform(const float *data, size_t count) {
  if (data != nullptr && count > 0) {
    shader.setUniform1fv("audio", data, std::min(count, (size_t)256));
  }
}
