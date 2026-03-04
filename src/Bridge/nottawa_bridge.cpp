//
//  nottawa_bridge.cpp
//  dude_wake_up
//
//  C bridging API implementation. All functions call into existing
//  C++ singleton services.
//

#include "nottawa_bridge.h"

#include "IOSurfaceTextureManager.hpp"
#include "ActionService.hpp"
#include "AudioSourceService.hpp"
#include "ConfigService.hpp"
#include "EngineLoop.hpp"
#include "LayoutStateService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "ShaderSource.hpp"
#include "BlendShader.hpp"
#include "OscillationService.hpp"
#include "WaveformOscillator.hpp"
#include "Strand.hpp"
#include "StrandService.hpp"
#include "Workspace.hpp"

#include <filesystem>
#include "LibraryService.hpp"
#include "LibraryFile.hpp"
#include "FileSource.hpp"
#include "FileAudioSource.hpp"
#include "TextSource.hpp"
#include "ofMain.h"

#include "httplib.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <set>
#include <string>
#include <thread>
#include <atomic>
#include <future>
#include <type_traits>

// ---------------------------------------------------------------------------
// MARK: - Engine Thread
// ---------------------------------------------------------------------------

static std::thread engineThread;
static std::atomic<bool> engineRunning{false};
static std::thread::id engineThreadId;

// Graph changed callback — set by ntw_register_graph_changed_callback.
static NTWGraphChangedCallback graphChangedCallback = nullptr;

static void notifyGraphChanged() {
  if (graphChangedCallback) graphChangedCallback();
}

static void engineThreadFunc() {
  try {
    engineThreadId = std::this_thread::get_id();
    EngineLoop::getInstance()->makeContextCurrent();
    using clock = std::chrono::steady_clock;
    using us = std::chrono::microseconds;
    constexpr auto frameDuration = us(16667); // ~60fps
    constexpr auto sleepMargin = us(2000);    // spin-wait the last 2ms
    while (engineRunning.load()) {
      auto frameStart = clock::now();
      EngineLoop::getInstance()->tick();
      auto remaining = frameDuration - std::chrono::duration_cast<us>(clock::now() - frameStart);
      if (remaining > sleepMargin) {
        std::this_thread::sleep_for(remaining - sleepMargin);
      }
      // Spin-wait for precision
      while (std::chrono::duration_cast<us>(clock::now() - frameStart) < frameDuration) {
        // busy wait
      }
    }
  } catch (const std::exception& e) {
    fprintf(stderr, "NTW: engine thread exception: %s\n", e.what());
  } catch (...) {
    fprintf(stderr, "NTW: engine thread unknown exception\n");
  }
  // Signal that the engine thread is no longer running so
  // runOnEngineThread() falls through instead of deadlocking.
  engineRunning.store(false);
}

// ---------------------------------------------------------------------------
// MARK: - Thread Dispatch Helper
// ---------------------------------------------------------------------------
// Mutations that create GL resources or modify shared state must run on the
// engine thread (which owns the OpenGL context). This helper dispatches a
// callable to the engine thread and blocks until it completes, returning the
// result. If already on the engine thread, or the engine isn't running yet,
// it executes directly to avoid deadlock.

template<typename F>
static auto runOnEngineThread(F&& func) -> decltype(func()) {
  using R = decltype(func());

  // Already on engine thread or engine not running — run directly.
  if (!engineRunning.load() ||
      std::this_thread::get_id() == engineThreadId) {
    return func();
  }

  // Dispatch to engine thread and block until complete.
  std::promise<R> prom;
  auto fut = prom.get_future();
  EngineLoop::getInstance()->executeOnEngineThread([&prom, &func]() {
    if constexpr (std::is_void_v<R>) {
      func();
      prom.set_value();
    } else {
      prom.set_value(func());
    }
  });
  return fut.get();
}

// ---------------------------------------------------------------------------
// MARK: - Helpers
// ---------------------------------------------------------------------------

static char* strdup_safe(const std::string& s) {
  return strdup(s.c_str());
}

static char* strdup_safe(const char* s) {
  return s ? strdup(s) : strdup("");
}


// Find a Connectable (shader or video source) by its string ID.
static std::shared_ptr<Connectable> findConnectable(const char* id) {
  if (!id) return nullptr;
  std::string sid(id);

  // Check video source first — videoSourceForId doesn't log on miss,
  // whereas shaderForId logs a spurious error for video source IDs.
  auto source = VideoSourceService::getService()->videoSourceForId(sid);
  if (source) return std::dynamic_pointer_cast<Connectable>(source);

  auto shader = ShaderChainerService::getService()->shaderForId(sid);
  if (shader) return shader;

  return nullptr;
}

// ---------------------------------------------------------------------------
// MARK: - Memory Management
// ---------------------------------------------------------------------------

void ntw_free_parameter_info(NTWParameterInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->name);
  if (info->driverName) free((void*)info->driverName);
  if (info->options) {
    for (int i = 0; i < info->optionCount; i++) {
      free((void*)info->options[i]);
    }
    free((void*)info->options);
  }
}

void ntw_free_parameter_info_array(NTWParameterInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_parameter_info(&array[i]);
  }
  free(array);
}

void ntw_free_connectable_info(NTWConnectableInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->name);
}

void ntw_free_connectable_info_array(NTWConnectableInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_connectable_info(&array[i]);
  }
  free(array);
}

void ntw_free_connection_info(NTWConnectionInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->startId);
  free((void*)info->endId);
}

void ntw_free_connection_info_array(NTWConnectionInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_connection_info(&array[i]);
  }
  free(array);
}

void ntw_free_available_shader_info(NTWAvailableShaderInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->name);
  if (info->category) free((void*)info->category);
}

void ntw_free_available_shader_info_array(NTWAvailableShaderInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_available_shader_info(&array[i]);
  }
  free(array);
}

void ntw_free_video_source_info(NTWVideoSourceInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->name);
}

void ntw_free_video_source_info_array(NTWVideoSourceInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_video_source_info(&array[i]);
  }
  free(array);
}

void ntw_free_audio_source_info(NTWAudioSourceInfo* info) {
  if (!info) return;
  free((void*)info->id);
  free((void*)info->name);
}

void ntw_free_audio_source_info_array(NTWAudioSourceInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    ntw_free_audio_source_info(&array[i]);
  }
  free(array);
}

void ntw_free_string(char* str) {
  free(str);
}

// ---------------------------------------------------------------------------
// MARK: - Engine Lifecycle
// ---------------------------------------------------------------------------

static void ntwTerminateHandler() {
  auto eptr = std::current_exception();
  if (eptr) {
    try {
      std::rethrow_exception(eptr);
    } catch (const std::exception& e) {
      fprintf(stderr, "\n*** NTW TERMINATE: %s\n", e.what());
    } catch (...) {
      fprintf(stderr, "\n*** NTW TERMINATE: unknown exception type\n");
    }
  } else {
    fprintf(stderr, "\n*** NTW TERMINATE: no active exception (direct std::terminate call)\n");
  }
  abort();
}

bool ntw_engine_init(void) {
  std::set_terminate(ntwTerminateHandler);
  EngineLoop::getInstance()->setup();
  return EngineLoop::getInstance()->isSetup();
}

void ntw_engine_start(void) {
  if (engineRunning.load()) return;
  EngineLoop::getInstance()->releaseContext();
  engineRunning.store(true);
  engineThread = std::thread(engineThreadFunc);
  // Detach so the static std::thread destructor doesn't call std::terminate
  // if the app exits without calling ntw_engine_stop().
  engineThread.detach();
}

void ntw_engine_stop(void) {
  if (!engineRunning.load()) return;
  engineRunning.store(false);
  // Thread is detached, so we just signal it to stop via the atomic flag.
  // Wait long enough for the current tick to finish (a tick at 60fps = ~16ms,
  // but shader rendering can take longer).
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void ntw_engine_tick(void) {
  EngineLoop::getInstance()->tick();
}

bool ntw_engine_is_initialized(void) {
  return EngineLoop::getInstance()->isSetup();
}

float ntw_engine_fps(void) {
  return EngineLoop::getInstance()->currentFPS();
}

// ---------------------------------------------------------------------------
// MARK: - Shader Mutations
// ---------------------------------------------------------------------------

char* ntw_add_shader(int shaderTypeRaw) {
  auto result = runOnEngineThread([=]() -> char* {
    auto shader = ActionService::getService()->addShader(static_cast<ShaderType>(shaderTypeRaw));
    if (!shader) return nullptr;
    return strdup_safe(shader->shaderId);
  });
  notifyGraphChanged();
  return result;
}

char* ntw_create_blend_between(const char* id1, const char* id2) {
  if (!id1 || !id2) return nullptr;
  std::string sid1(id1), sid2(id2);

  auto result = runOnEngineThread([sid1, sid2]() -> char* {
    auto conn1 = findConnectable(sid1.c_str());
    auto conn2 = findConnectable(sid2.c_str());
    if (!conn1 || !conn2) return nullptr;

    auto blend = ActionService::getService()->addShader(ShaderTypeBlend);
    if (!blend) return nullptr;

    // Connect both inputs to the blend shader.
    // Use ConnectionTypeSource if the input is a video source.
    auto type1 = conn1->connectableType() == ConnectableTypeSource
      ? ConnectionTypeSource : ConnectionTypeShader;
    auto type2 = conn2->connectableType() == ConnectableTypeSource
      ? ConnectionTypeSource : ConnectionTypeShader;
    ActionService::getService()->makeConnection(
      conn1, blend, type1, OutputSlotMain, InputSlotMain, true);
    ActionService::getService()->makeConnection(
      conn2, blend, type2, OutputSlotMain, InputSlotTwo, true);

    return strdup_safe(blend->shaderId);
  });
  notifyGraphChanged();
  return result;
}

char* ntw_replace_shader(const char* shaderId, int newShaderTypeRaw) {
  if (!shaderId) return nullptr;
  std::string sid(shaderId);

  auto result = runOnEngineThread([sid, newShaderTypeRaw]() -> char* {
    auto shader = ShaderChainerService::getService()->shaderForId(sid);
    if (!shader) return nullptr;

    auto newShader = ShaderChainerService::getService()->replaceShader(
      shader, static_cast<ShaderType>(newShaderTypeRaw));
    if (!newShader) return nullptr;

    return strdup_safe(newShader->shaderId);
  });
  notifyGraphChanged();
  return result;
}

char* ntw_get_shader_file_path(const char* shaderId) {
  if (!shaderId) return nullptr;
  std::string sid(shaderId);

  auto shader = ShaderChainerService::getService()->shaderForId(sid);
  if (!shader) return nullptr;

  std::string path = ofToDataPath("shaders/" + shader->fileName() + ".frag");
  return strdup_safe(path);
}

bool ntw_supports_aux_output(const char* shaderId) {
  if (!shaderId) return false;
  auto shader = ShaderChainerService::getService()->shaderForId(std::string(shaderId));
  if (!shader) return false;
  return shader->allowAuxOutputSlot();
}

void ntw_remove_shader(const char* shaderId) {
  if (!shaderId) return;
  std::string sid(shaderId);
  runOnEngineThread([sid]() {
    auto shader = ShaderChainerService::getService()->shaderForId(sid);
    if (shader) {
      ActionService::getService()->removeShader(shader);
    }
  });
  notifyGraphChanged();
}

// ---------------------------------------------------------------------------
// MARK: - Video Source Mutations
// ---------------------------------------------------------------------------

char* ntw_add_shader_video_source(int shaderSourceTypeRaw) {
  auto result = runOnEngineThread([=]() -> char* {
    auto source = ActionService::getService()->addShaderVideoSource(
      static_cast<ShaderSourceType>(shaderSourceTypeRaw));
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
  notifyGraphChanged();
  return result;
}

char* ntw_add_text_video_source(const char* name) {
  std::string sname = name ? std::string(name) : "Text";
  auto result = runOnEngineThread([sname]() -> char* {
    auto source = ActionService::getService()->addTextVideoSource(sname);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
  notifyGraphChanged();
  return result;
}

char* ntw_add_image_video_source(const char* name, const char* path) {
  std::string sname = name ? std::string(name) : "Image";
  std::string spath = path ? std::string(path) : "";
  return runOnEngineThread([sname, spath]() -> char* {
    auto source = ActionService::getService()->addImageVideoSource(sname, spath);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
}

char* ntw_add_file_video_source(const char* name, const char* path) {
  std::string sname = name ? std::string(name) : "Video";
  std::string spath = path ? std::string(path) : "";
  return runOnEngineThread([sname, spath]() -> char* {
    auto source = ActionService::getService()->addFileVideoSource(sname, spath);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
}

char* ntw_add_webcam_video_source(const char* name, int deviceId) {
  std::string sname = name ? std::string(name) : "Webcam";
  return runOnEngineThread([sname, deviceId]() -> char* {
    auto source = ActionService::getService()->addWebcamVideoSource(sname, deviceId);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
}

char* ntw_add_icon_video_source(const char* name) {
  std::string sname = name ? std::string(name) : "Icon";
  return runOnEngineThread([sname]() -> char* {
    auto source = ActionService::getService()->addIconVideoSource(sname);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
}

char* ntw_add_playlist_video_source(const char* name) {
  std::string sname = name ? std::string(name) : "Playlist";
  return runOnEngineThread([sname]() -> char* {
    auto source = ActionService::getService()->addPlaylistVideoSource(sname);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
}

void ntw_remove_video_source(const char* sourceId) {
  if (!sourceId) return;
  std::string sid(sourceId);
  runOnEngineThread([sid]() {
    auto source = VideoSourceService::getService()->videoSourceForId(sid);
    if (source) {
      ActionService::getService()->removeVideoSource(source);
    }
  });
}

char* ntw_replace_video_source(const char* videoSourceId, int shaderSourceTypeRaw) {
  if (!videoSourceId) return nullptr;
  std::string sid(videoSourceId);

  auto result = runOnEngineThread([sid, shaderSourceTypeRaw]() -> char* {
    auto source = VideoSourceService::getService()->videoSourceForId(sid);
    if (!source) return nullptr;

    auto shaderSource = std::dynamic_pointer_cast<ShaderSource>(source);
    if (!shaderSource) return nullptr;

    auto newSource = VideoSourceService::getService()->replaceShaderVideoSource(
      shaderSource, static_cast<ShaderSourceType>(shaderSourceTypeRaw));
    if (!newSource) return nullptr;

    return strdup_safe(newSource->id);
  });
  notifyGraphChanged();
  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Connection Mutations
// ---------------------------------------------------------------------------

char* ntw_make_connection(const char* startId,
                          const char* endId,
                          int connectionType,
                          int outputSlot,
                          int inputSlot) {
  if (!startId || !endId) return nullptr;
  std::string sStart(startId), sEnd(endId);

  auto result = runOnEngineThread([sStart, sEnd, connectionType, outputSlot, inputSlot]() -> char* {
    auto start = findConnectable(sStart.c_str());
    auto end = findConnectable(sEnd.c_str());
    if (!start || !end) return nullptr;

    auto connection = ActionService::getService()->makeConnection(
      start, end,
      static_cast<ConnectionType>(connectionType),
      static_cast<OutputSlot>(outputSlot),
      static_cast<InputSlot>(inputSlot),
      true);

    if (!connection) {
      return nullptr;
    }
    return strdup_safe(connection->id);
  });
  notifyGraphChanged();
  return result;
}

void ntw_remove_connection(const char* connectionId) {
  if (!connectionId) return;
  std::string sid(connectionId);
  runOnEngineThread([sid]() {
    auto connections = ShaderChainerService::getService()->connections();
    for (auto& conn : connections) {
      if (conn->id == sid) {
        ActionService::getService()->removeConnection(conn);
        return;
      }
    }
  });
  notifyGraphChanged();
}

// ---------------------------------------------------------------------------
// MARK: - Undo / Redo
// ---------------------------------------------------------------------------

void ntw_undo(void) {
  runOnEngineThread([]() { ActionService::getService()->undo(); });
  notifyGraphChanged();
}

void ntw_redo(void) {
  runOnEngineThread([]() { ActionService::getService()->redo(); });
  notifyGraphChanged();
}

bool ntw_can_undo(void) {
  return ActionService::getService()->canUndo();
}

bool ntw_can_redo(void) {
  return ActionService::getService()->canRedo();
}

void ntw_copy_connectables(const char** ids, int count) {
  // Capture the IDs as strings before dispatching.
  std::vector<std::string> idStrings;
  for (int i = 0; i < count; i++) {
    if (ids[i]) idStrings.emplace_back(ids[i]);
  }
  runOnEngineThread([idStrings]() {
    std::vector<std::shared_ptr<Connectable>> connectables;
    for (const auto& sid : idStrings) {
      auto c = findConnectable(sid.c_str());
      if (c) connectables.push_back(c);
    }
    if (!connectables.empty()) {
      ActionService::getService()->copy(connectables);
    }
  });
}

char** ntw_paste_connectables(int* outCount) {
  *outCount = 0;
  auto result = runOnEngineThread([&]() -> char** {
    auto copied = ActionService::getService()->getCopiedConnectables();
    if (copied.empty()) return nullptr;

    auto newConnectables = ShaderChainerService::getService()->pasteConnectables(copied);
    if (newConnectables.empty()) return nullptr;

    // Apply +40/+40 offset so pasted nodes don't overlap originals.
    for (auto& c : newConnectables) {
      if (c->connectableType() == ConnectableTypeShader) {
        auto shader = std::dynamic_pointer_cast<Shader>(c);
        if (shader) {
          shader->settings->x->setValue(shader->settings->x->value + 40.0f);
          shader->settings->y->setValue(shader->settings->y->value + 40.0f);
        }
      } else if (c->connectableType() == ConnectableTypeSource) {
        auto source = std::dynamic_pointer_cast<VideoSource>(c);
        if (source) {
          source->origin.x += 40.0f;
          source->origin.y += 40.0f;
        }
      }
    }

    int count = static_cast<int>(newConnectables.size());
    *outCount = count;
    char** ids = (char**)malloc(sizeof(char*) * count);
    for (int i = 0; i < count; i++) {
      ids[i] = strdup_safe(newConnectables[i]->connId());
    }
    return ids;
  });
  notifyGraphChanged();
  return result;
}

bool ntw_has_copied_connectables(void) {
  return !ActionService::getService()->getCopiedConnectables().empty();
}

// ---------------------------------------------------------------------------
// MARK: - Queries: Shaders
// ---------------------------------------------------------------------------

NTWConnectableInfo* ntw_get_all_shaders(int* outCount) {
  auto shaders = ShaderChainerService::getService()->shaders();
  int count = static_cast<int>(shaders.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWConnectableInfo*>(
    calloc(count, sizeof(NTWConnectableInfo)));

  for (int i = 0; i < count; i++) {
    auto& s = shaders[i];
    result[i].id = strdup_safe(s->shaderId);
    result[i].name = strdup_safe(s->name());
    result[i].shaderTypeRaw = static_cast<int>(s->type());
    result[i].connectableType = 1; // Shader
    result[i].inputCount = s->inputCount();
    result[i].posX = s->settings->x ? s->settings->x->value : 0.0f;
    result[i].posY = s->settings->y ? s->settings->y->value : 0.0f;
  }

  return result;
}

NTWConnectableInfo ntw_get_shader(const char* shaderId) {
  NTWConnectableInfo info = {};
  if (!shaderId) return info;

  auto shader = ShaderChainerService::getService()->shaderForId(std::string(shaderId));
  if (!shader) return info;

  info.id = strdup_safe(shader->shaderId);
  info.name = strdup_safe(shader->name());
  info.shaderTypeRaw = static_cast<int>(shader->type());
  info.connectableType = 1;
  info.inputCount = shader->inputCount();
  info.posX = shader->settings->x ? shader->settings->x->value : 0.0f;
  info.posY = shader->settings->y ? shader->settings->y->value : 0.0f;

  return info;
}

// ---------------------------------------------------------------------------
// MARK: - Queries: Video Sources
// ---------------------------------------------------------------------------

NTWVideoSourceInfo* ntw_get_all_video_sources(int* outCount) {
  auto sources = VideoSourceService::getService()->videoSources();
  int count = static_cast<int>(sources.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWVideoSourceInfo*>(
    calloc(count, sizeof(NTWVideoSourceInfo)));

  for (int i = 0; i < count; i++) {
    auto& src = sources[i];
    result[i].id = strdup_safe(src->id);
    result[i].name = strdup_safe(src->sourceName);
    result[i].sourceType = static_cast<int>(src->type);
    result[i].inputCount = src->inputCount();
    result[i].posX = src->origin.x;
    result[i].posY = src->origin.y;
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Queries: Connections
// ---------------------------------------------------------------------------

NTWConnectionInfo* ntw_get_all_connections(int* outCount) {
  auto connections = ShaderChainerService::getService()->connections();
  int count = static_cast<int>(connections.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWConnectionInfo*>(
    calloc(count, sizeof(NTWConnectionInfo)));

  for (int i = 0; i < count; i++) {
    auto& conn = connections[i];
    result[i].id = strdup_safe(conn->id);
    result[i].startId = strdup_safe(conn->start ? conn->start->connId() : "");
    result[i].endId = strdup_safe(conn->end ? conn->end->connId() : "");
    result[i].connectionType = static_cast<int>(conn->type);
    result[i].inputSlot = static_cast<int>(conn->inputSlot);
    result[i].outputSlot = static_cast<int>(conn->outputSlot);
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Queries: Connections for Connectable
// ---------------------------------------------------------------------------

NTWConnectionInfo* ntw_get_connections_for_connectable(const char* connectableId, int* outCount) {
  if (outCount) *outCount = 0;
  if (!connectableId) return nullptr;
  std::string sid(connectableId);

  auto allConns = ShaderChainerService::getService()->connections();
  std::vector<std::shared_ptr<Connection>> matching;
  for (auto& conn : allConns) {
    if ((conn->start && conn->start->connId() == sid) ||
        (conn->end && conn->end->connId() == sid)) {
      matching.push_back(conn);
    }
  }

  int count = static_cast<int>(matching.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWConnectionInfo*>(
    calloc(count, sizeof(NTWConnectionInfo)));

  for (int i = 0; i < count; i++) {
    auto& conn = matching[i];
    result[i].id = strdup_safe(conn->id);
    result[i].startId = strdup_safe(conn->start ? conn->start->connId() : "");
    result[i].endId = strdup_safe(conn->end ? conn->end->connId() : "");
    result[i].connectionType = static_cast<int>(conn->type);
    result[i].inputSlot = static_cast<int>(conn->inputSlot);
    result[i].outputSlot = static_cast<int>(conn->outputSlot);
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Helpers: Populate Oscillator/Driver Fields
// ---------------------------------------------------------------------------

static void populateOscillatorDriverFields(NTWParameterInfo& info, const std::shared_ptr<Parameter>& p) {
  // Oscillator lookup
  auto osc = OscillationService::getService()->oscillatorForParameter(p);
  auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
  {
    FILE* dbg = fopen("/tmp/ntw_osc_debug.log", "a");
    if (dbg) {
      fprintf(dbg, "param '%s' (id=%s): osc=%p waveOsc=%p, total_oscs=%zu\n",
              p->name.c_str(), p->paramId.c_str(), (void*)osc.get(), (void*)waveOsc.get(),
              OscillationService::getService()->oscillators.size());
      fclose(dbg);
    }
  }
  if (waveOsc) {
    info.hasOscillator = 1;
    info.oscillatorEnabled = (waveOsc->enabled && waveOsc->enabled->boolValue) ? 1 : 0;
    info.oscillatorFrequency = waveOsc->frequency ? waveOsc->frequency->value : 1.0f;
    info.oscillatorMinOutput = waveOsc->minOutput ? waveOsc->minOutput->value : p->min;
    info.oscillatorMaxOutput = waveOsc->maxOutput ? waveOsc->maxOutput->value : p->max;
    info.oscillatorWaveShape = waveOsc->waveShape ? static_cast<int>(waveOsc->waveShape->value) : 0;
  } else {
    info.hasOscillator = 0;
    info.oscillatorEnabled = 0;
    info.oscillatorFrequency = 1.0f;
    info.oscillatorMinOutput = p->min;
    info.oscillatorMaxOutput = p->max;
    info.oscillatorWaveShape = 0;
  }

  // Audio driver lookup
  if (p->driver != nullptr) {
    info.hasDriver = 1;
    info.driverName = strdup_safe(p->driver->name);
    info.driverShift = p->shift ? p->shift->value : 0.5f;
    info.driverScale = p->scale ? p->scale->value : 0.2f;
  } else {
    info.hasDriver = 0;
    info.driverName = nullptr;
    info.driverShift = 0.5f;
    info.driverScale = 0.2f;
  }
}

// ---------------------------------------------------------------------------
// MARK: - Queries: Parameters
// ---------------------------------------------------------------------------

NTWParameterInfo* ntw_get_parameters(const char* connectableId, int* outCount) {
  if (outCount) *outCount = 0;
  if (!connectableId) return nullptr;

  auto connectable = findConnectable(connectableId);
  if (!connectable) return nullptr;

  auto settings = connectable->settingsRef();
  if (!settings) return nullptr;

  auto& params = settings->parameters;
  int count = static_cast<int>(params.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWParameterInfo*>(
    calloc(count, sizeof(NTWParameterInfo)));

  for (int i = 0; i < count; i++) {
    auto& p = params[i];
    if (!p) continue;

    result[i].id = strdup_safe(p->paramId);
    result[i].name = strdup_safe(p->name);
    result[i].value = p->value;
    result[i].minValue = p->min;
    result[i].maxValue = p->max;
    result[i].parameterType = static_cast<int>(p->type);
    result[i].intValue = p->intValue;
    result[i].boolValue = p->boolValue ? 1 : 0;
    result[i].isFavorited = p->favorited ? 1 : 0;
    result[i].colorR = p->color->at(0);
    result[i].colorG = p->color->at(1);
    result[i].colorB = p->color->at(2);
    result[i].colorA = p->color->at(3);
    populateOscillatorDriverFields(result[i], p);

    // Populate options (e.g. font names for font selector)
    if (!p->options.empty()) {
      int optCount = static_cast<int>(p->options.size());
      result[i].optionCount = optCount;
      result[i].options = static_cast<const char**>(calloc(optCount, sizeof(const char*)));
      for (int j = 0; j < optCount; j++) {
        result[i].options[j] = strdup_safe(p->options[j]);
      }
    } else {
      result[i].optionCount = 0;
      result[i].options = nullptr;
    }
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Parameter Control
// ---------------------------------------------------------------------------

void ntw_set_parameter_value(const char* paramId, float value) {
  if (!paramId) return;
  auto param = ParameterService::getService()->parameterForId(std::string(paramId));
  if (param) param->setValue(value);
}

void ntw_set_parameter_int(const char* paramId, int value) {
  if (!paramId) return;
  auto param = ParameterService::getService()->parameterForId(std::string(paramId));
  if (param) {
    param->intValue = value;
    param->affirmIntValue();
  }
}

void ntw_set_parameter_bool(const char* paramId, bool value) {
  if (!paramId) return;
  auto param = ParameterService::getService()->parameterForId(std::string(paramId));
  if (param) param->setBoolValue(value);
}

void ntw_set_parameter_color(const char* paramId,
                             float r, float g, float b, float a) {
  if (!paramId) return;
  auto param = ParameterService::getService()->parameterForId(std::string(paramId));
  if (param) {
    param->setColor({r, g, b, a});
  }
}

void ntw_toggle_parameter_favorite(const char* paramId) {
  if (!paramId) return;
  auto param = ParameterService::getService()->parameterForId(std::string(paramId));
  if (!param) return;

  if (param->favorited) {
    ParameterService::getService()->removeFavoriteParameter(param);
    param->favorited = false;
  } else {
    ParameterService::getService()->addFavoriteParameter(param);
    param->favorited = true;
  }
}

// ---------------------------------------------------------------------------
// MARK: - Oscillator Control
// ---------------------------------------------------------------------------

void ntw_set_oscillator_enabled(const char* paramId, bool enabled) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, enabled]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto osc = OscillationService::getService()->oscillatorForParameter(param);
    if (osc && osc->enabled) osc->enabled->setBoolValue(enabled);
  });
}

void ntw_set_oscillator_frequency(const char* paramId, float frequency) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, frequency]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto osc = OscillationService::getService()->oscillatorForParameter(param);
    auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
    if (waveOsc && waveOsc->frequency) waveOsc->frequency->setValue(frequency);
  });
}

void ntw_set_oscillator_min_output(const char* paramId, float minOutput) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, minOutput]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto osc = OscillationService::getService()->oscillatorForParameter(param);
    auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
    if (waveOsc && waveOsc->minOutput) waveOsc->minOutput->setValue(minOutput);
  });
}

void ntw_set_oscillator_max_output(const char* paramId, float maxOutput) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, maxOutput]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto osc = OscillationService::getService()->oscillatorForParameter(param);
    auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
    if (waveOsc && waveOsc->maxOutput) waveOsc->maxOutput->setValue(maxOutput);
  });
}

void ntw_set_oscillator_wave_shape(const char* paramId, int waveShape) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, waveShape]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto osc = OscillationService::getService()->oscillatorForParameter(param);
    auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
    if (waveOsc && waveOsc->waveShape) waveOsc->waveShape->setValue(static_cast<float>(waveShape));
  });
}

// ---------------------------------------------------------------------------
// MARK: - Audio Driver Control
// ---------------------------------------------------------------------------

void ntw_set_parameter_driver(const char* paramId, const char* audioParamName) {
  if (!paramId || !audioParamName) return;
  std::string sid(paramId);
  std::string driverName(audioParamName);
  runOnEngineThread([sid, driverName]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (!source) return;
    for (auto& ap : source->audioAnalysis.parameters) {
      if (ap->name == driverName) {
        param->addDriver(ap);
        return;
      }
    }
  });
}

void ntw_remove_parameter_driver(const char* paramId) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param) return;
    param->removeDriver();
  });
}

void ntw_set_parameter_driver_shift(const char* paramId, float shift) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, shift]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param || !param->shift) return;
    param->shift->setValue(shift);
  });
}

void ntw_set_parameter_driver_scale(const char* paramId, float scale) {
  if (!paramId) return;
  std::string sid(paramId);
  runOnEngineThread([sid, scale]() {
    auto param = ParameterService::getService()->parameterForId(sid);
    if (!param || !param->scale) return;
    param->scale->setValue(scale);
  });
}

NTWAudioDriverParam* ntw_get_available_audio_drivers(int* outCount) {
  if (outCount) *outCount = 0;
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return nullptr;

  auto& params = source->audioAnalysis.parameters;
  int count = static_cast<int>(params.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAudioDriverParam*>(
    calloc(count, sizeof(NTWAudioDriverParam)));
  for (int i = 0; i < count; i++) {
    result[i].name = strdup_safe(params[i]->name);
    result[i].index = i;
  }
  return result;
}

void ntw_free_audio_driver_param_array(NTWAudioDriverParam* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].name);
  }
  free(array);
}

// ---------------------------------------------------------------------------
// MARK: - Available Shaders (Browser)
// ---------------------------------------------------------------------------

NTWAvailableShaderInfo* ntw_get_available_shaders(int category, int* outCount) {
  if (outCount) *outCount = 0;
  auto* svc = ShaderChainerService::getService();

  std::vector<std::shared_ptr<AvailableShader>>* list = nullptr;
  switch (category) {
    case 0: list = &svc->allAvailableShaders; break;
    case 1: list = &svc->availableBasicShaders; break;
    case 2: list = &svc->availableMixShaders; break;
    case 3: list = &svc->availableTransformShaders; break;
    case 4: list = &svc->availableFilterShaders; break;
    case 5: list = &svc->availableMaskShaders; break;
    case 6: list = &svc->availableGlitchShaders; break;
    default: return nullptr;
  }

  // Build set of ShaderTypes that are also ShaderSource types (generators).
  // These belong in the Sources tab, not the Shaders (effects) tab.
  static std::set<ShaderType> sourceShaderTypes = []() {
    std::set<ShaderType> s;
    for (auto sst : AvailableShaderSourceTypes) {
      s.insert(shaderTypeForShaderSourceType(sst));
    }
    return s;
  }();

  // Filter to only effect shaders (exclude source shader types).
  std::vector<std::shared_ptr<AvailableShader>> filtered;
  for (auto& as : *list) {
    if (sourceShaderTypes.find(as->type) == sourceShaderTypes.end()) {
      filtered.push_back(as);
    }
  }

  int count = static_cast<int>(filtered.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAvailableShaderInfo*>(
    calloc(count, sizeof(NTWAvailableShaderInfo)));

  auto* paramSvc = ParameterService::getService();
  for (int i = 0; i < count; i++) {
    auto& as = filtered[i];
    result[i].id = strdup_safe(std::to_string(static_cast<int>(as->type)));
    result[i].name = strdup_safe(as->name);
    result[i].category = nullptr;
    result[i].shaderTypeRaw = static_cast<int>(as->type);
    result[i].isFavorited = paramSvc->isShaderTypeFavorited(as->type) ? 1 : 0;
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Node Position & Active State
// ---------------------------------------------------------------------------

void ntw_set_connectable_position(const char* connectableId, float x, float y) {
  if (!connectableId) return;
  std::string sid(connectableId);
  runOnEngineThread([sid, x, y]() {
    // Try video source first (avoids spurious log from shaderForId).
    auto source = VideoSourceService::getService()->videoSourceForId(sid);
    if (source) {
      source->origin.x = x;
      source->origin.y = y;
      return;
    }
    auto shader = ShaderChainerService::getService()->shaderForId(sid);
    if (shader) {
      if (shader->settings->x) shader->settings->x->setValue(x);
      if (shader->settings->y) shader->settings->y->setValue(y);
    }
  });
}

void ntw_set_connectable_active(const char* connectableId, bool active) {
  if (!connectableId) return;
  std::string sid(connectableId);
  auto conn = findConnectable(sid.c_str());
  if (conn) conn->active = active;
}

bool ntw_get_connectable_active(const char* connectableId) {
  if (!connectableId) return false;
  auto conn = findConnectable(connectableId);
  return conn ? conn->active : false;
}

void ntw_set_shader_bypassed(const char* shaderId, int bypassed) {
  if (!shaderId) return;
  std::string sid(shaderId);
  auto shader = ShaderChainerService::getService()->shaderForId(sid);
  if (shader) shader->bypassed = (bypassed != 0);
}

int ntw_get_shader_bypassed(const char* shaderId) {
  if (!shaderId) return 0;
  std::string sid(shaderId);
  auto shader = ShaderChainerService::getService()->shaderForId(sid);
  return shader ? (shader->bypassed ? 1 : 0) : 0;
}

int ntw_get_shader_depth(const char* shaderId) {
  if (!shaderId) return 0;
  std::string sid(shaderId);
  auto shader = ShaderChainerService::getService()->shaderForId(sid);
  if (!shader) return 0;

  // Walk input connections back to source, counting depth
  int depth = 0;
  auto current = std::dynamic_pointer_cast<Connectable>(shader);
  while (current && !current->inputs.empty()) {
    auto it = current->inputs.find(InputSlotMain);
    if (it == current->inputs.end()) break;
    auto parent = it->second->start;
    if (!parent) break;
    depth++;
    if (parent->connectableType() == ConnectableTypeSource) break;
    current = parent;
  }
  return depth;
}

void ntw_remove_connectable(const char* connectableId) {
  if (!connectableId) return;
  std::string sid(connectableId);
  runOnEngineThread([sid]() {
    // Try video source first (avoids spurious log from shaderForId).
    auto source = VideoSourceService::getService()->videoSourceForId(sid);
    if (source) {
      ActionService::getService()->removeVideoSource(source);
      return;
    }
    auto shader = ShaderChainerService::getService()->shaderForId(sid);
    if (shader) {
      ActionService::getService()->removeShader(shader);
    }
  });
  notifyGraphChanged();
}

// ---------------------------------------------------------------------------
// MARK: - Available Shader Sources (Browser)
// ---------------------------------------------------------------------------

NTWAvailableShaderInfo* ntw_get_available_shader_sources(int* outCount) {
  if (outCount) *outCount = 0;

  int count = sizeof(AvailableShaderSourceTypes) / sizeof(AvailableShaderSourceTypes[0]);
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAvailableShaderInfo*>(
    calloc(count, sizeof(NTWAvailableShaderInfo)));

  auto* paramSvc = ParameterService::getService();
  for (int i = 0; i < count; i++) {
    auto type = AvailableShaderSourceTypes[i];
    std::string name = shaderSourceTypeName(type);
    std::string cat = shaderSourceTypeCategory(type);
    result[i].id = strdup_safe("src_" + std::to_string(static_cast<int>(type)));
    result[i].name = strdup_safe(name);
    result[i].category = strdup_safe(cat);
    result[i].shaderTypeRaw = static_cast<int>(type);
    result[i].isFavorited = paramSvc->isFavoriteSourceType(static_cast<int>(type)) ? 1 : 0;
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Selection
// ---------------------------------------------------------------------------

void ntw_select_connectable(const char* connectableId) {
  if (!connectableId) return;
  auto conn = findConnectable(connectableId);
  if (conn) {
    ShaderChainerService::getService()->selectConnectable(conn);
  }
}

void ntw_deselect_connectable(void) {
  ShaderChainerService::getService()->deselectConnectable();
}

char* ntw_get_selected_connectable_id(void) {
  auto selected = ShaderChainerService::getService()->selectedConnectable;
  if (!selected) return nullptr;
  return strdup_safe(selected->connId());
}

// ---------------------------------------------------------------------------
// MARK: - Audio
// ---------------------------------------------------------------------------

NTWAudioAnalysisInfo ntw_get_audio_analysis(void) {
  NTWAudioAnalysisInfo info = {};
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return info;

  auto& analysis = source->audioAnalysis;
  info.rms = analysis.rms ? analysis.rms->value : 0.0f;
  info.lows = analysis.lows ? analysis.lows->value : 0.0f;
  info.mids = analysis.mids ? analysis.mids->value : 0.0f;
  info.highs = analysis.highs ? analysis.highs->value : 0.0f;
  info.bpm = analysis.bpm ? analysis.bpm->value : 0.0f;
  info.beatPulse = analysis.beatPulse ? analysis.beatPulse->value : 0.0f;
  info.beatDetected = false; // Snapshot; real beat detection is event-driven

  return info;
}

float ntw_get_current_bpm(void) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return 120.0f;
  return source->audioAnalysis.bpm ? source->audioAnalysis.bpm->value : 120.0f;
}

NTWAudioSourceInfo* ntw_get_all_audio_sources(int* outCount) {
  auto sources = AudioSourceService::getService()->audioSources();
  int count = static_cast<int>(sources.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAudioSourceInfo*>(
    calloc(count, sizeof(NTWAudioSourceInfo)));

  for (int i = 0; i < count; i++) {
    auto& src = sources[i];
    result[i].id = strdup_safe(src->id);
    result[i].name = strdup_safe(src->name);
    result[i].sourceType = static_cast<int>(src->type());
  }

  return result;
}

void ntw_select_audio_source(const char* audioSourceId) {
  if (!audioSourceId) return;
  auto source = AudioSourceService::getService()->audioSourceForId(std::string(audioSourceId));
  if (source) {
    AudioSourceService::getService()->selectAudioSource(source);
  }
}

// ---------------------------------------------------------------------------
// MARK: - Extended Audio (Audio Panel)
// ---------------------------------------------------------------------------

NTWExtendedAudioAnalysisInfo ntw_get_extended_audio_analysis(void) {
  NTWExtendedAudioAnalysisInfo info = {};
  auto svc = AudioSourceService::getService();
  auto source = svc->selectedAudioSource;
  if (!source) return info;

  auto& analysis = source->audioAnalysis;

  // Core values
  info.rms = analysis.rms ? analysis.rms->value : 0.0f;
  info.lows = analysis.lows ? analysis.lows->value : 0.0f;
  info.mids = analysis.mids ? analysis.mids->value : 0.0f;
  info.highs = analysis.highs ? analysis.highs->value : 0.0f;
  info.bpm = analysis.bpm ? analysis.bpm->value : 120.0f;
  info.beatPulse = analysis.beatPulse ? analysis.beatPulse->value : 0.0f;
  info.beatCount = analysis.beatCount;

  // BPM settings
  info.bpmMode = static_cast<int>(analysis.bpmMode);
  info.bpmLocked = analysis.bpmLocked ? analysis.bpmLocked->boolValue : false;
  info.bpmNudge = analysis.bpmNudge ? analysis.bpmNudge->value : 0.0f;
  info.bpmEnabled = analysis.bpmEnabled;

  // Frequency settings
  info.smoothingMode = static_cast<int>(analysis.smoothingMode);
  info.frequencyRelease = analysis.frequencyRelease ? analysis.frequencyRelease->value : 0.7f;
  info.frequencyScale = analysis.frequencyScale ? analysis.frequencyScale->value : 1.0f;
  info.loudnessRelease = analysis.rmsAnalysisParam.release ? analysis.rmsAnalysisParam.release->value : 0.6f;
  info.loudnessScale = analysis.loudnessScale ? analysis.loudnessScale->value : 1.0f;

  // State
  info.audioActive = source->active;
  info.audioSourceType = static_cast<int>(source->type());

  // Mel spectrum (copy up to 13 bins)
  auto melSpec = analysis.getSafeMelSpectrum();
  info.melSpectrumCount = std::min(static_cast<int>(melSpec.size()), 13);
  for (int i = 0; i < info.melSpectrumCount; i++) {
    info.melSpectrum[i] = melSpec[i];
  }

  // Waveform (downsample to 256 if needed)
  auto wave = analysis.getSafeWaveform();
  if (wave.empty()) {
    info.waveformCount = 0;
  } else if (wave.size() <= 256) {
    info.waveformCount = static_cast<int>(wave.size());
    for (int i = 0; i < info.waveformCount; i++) {
      info.waveform[i] = wave[i];
    }
  } else {
    // Downsample
    info.waveformCount = 256;
    float step = static_cast<float>(wave.size()) / 256.0f;
    for (int i = 0; i < 256; i++) {
      info.waveform[i] = wave[static_cast<int>(i * step)];
    }
  }

  return info;
}

NTWAudioTrackInfo* ntw_get_sample_tracks(int* outCount) {
  auto& tracks = AudioSourceService::getService()->sampleTracks;
  int count = static_cast<int>(tracks.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAudioTrackInfo*>(
    calloc(count, sizeof(NTWAudioTrackInfo)));

  for (int i = 0; i < count; i++) {
    result[i].name = strdup_safe(tracks[i]->name);
    result[i].path = strdup_safe(tracks[i]->path);
    result[i].bpm = tracks[i]->bpm;
    result[i].index = i;
  }

  return result;
}

void ntw_free_audio_track_info_array(NTWAudioTrackInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free(const_cast<char*>(array[i].name));
    free(const_cast<char*>(array[i].path));
  }
  free(array);
}

NTWFileAudioState ntw_get_file_audio_state(void) {
  NTWFileAudioState state = {};
  auto svc = AudioSourceService::getService();
  auto source = svc->selectedAudioSource;
  if (!source) return state;

  auto fileSrc = std::dynamic_pointer_cast<FileAudioSource>(source);
  if (!fileSrc) {
    state.isFileSource = false;
    return state;
  }

  state.isFileSource = true;
  state.volume = fileSrc->volume;
  state.isPaused = fileSrc->isPaused.load();
  state.playbackPosition = fileSrc->getPlaybackPosition();
  state.totalDuration = fileSrc->getTotalDuration();

  // Find selected track index
  auto& tracks = svc->sampleTracks;
  auto selectedTrack = svc->selectedSampleTrack;
  state.selectedTrackIndex = -1;
  if (selectedTrack) {
    for (int i = 0; i < static_cast<int>(tracks.size()); i++) {
      if (tracks[i] == selectedTrack) {
        state.selectedTrackIndex = i;
        break;
      }
    }
  }

  return state;
}

const char* ntw_get_selected_audio_source_id(void) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return nullptr;
  return strdup_safe(source->id);
}

void ntw_toggle_audio_source(void) {
  runOnEngineThread([]() {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (source) {
      source->toggle();
    }
  });
}

void ntw_set_bpm_mode(int mode) {
  runOnEngineThread([mode]() {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (!source) return;
    source->audioAnalysis.bpmMode = static_cast<BpmMode>(mode);
    source->audioAnalysis.bpmModeParam->intValue = mode;
  });
}

void ntw_set_bpm_locked(bool locked) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  source->audioAnalysis.bpmLocked->boolValue = locked;
}

void ntw_set_bpm_value(float bpm) {
  runOnEngineThread([bpm]() {
    auto svc = AudioSourceService::getService();
    auto source = svc->selectedAudioSource;
    if (!source) return;
    source->audioAnalysis.adjustBpmStartTimeForPhase(bpm);
    svc->tapper.setBpm(bpm);
  });
}

void ntw_nudge_bpm(float delta) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source || !source->audioAnalysis.bpmNudge) return;
  float newVal = source->audioAnalysis.bpmNudge->value + delta;
  newVal = std::max(-1.0f, std::min(1.0f, newVal));
  source->audioAnalysis.bpmNudge->value = newVal;
}

void ntw_set_bpm_nudge(float value) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source || !source->audioAnalysis.bpmNudge) return;
  source->audioAnalysis.bpmNudge->value = std::max(-1.0f, std::min(1.0f, value));
}

void ntw_tap_bpm(void) {
  runOnEngineThread([]() {
    auto svc = AudioSourceService::getService();
    auto source = svc->selectedAudioSource;
    if (!source) return;
    // If tapper has been idle, reset it
    if (svc->tapper.bpm() < 1.0f) {
      svc->tapper.startFresh();
    }
    svc->tapper.tap();
    float bpm = svc->tapper.bpm();
    if (bpm > 0) {
      source->audioAnalysis.adjustBpmStartTimeForPhase(bpm);
    }
  });
}

void ntw_set_bpm_enabled(bool enabled) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  source->audioAnalysis.bpmEnabled = enabled;
}

void ntw_set_smoothing_mode(int mode) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  source->audioAnalysis.smoothingMode = static_cast<SmoothingMode>(mode);
}

void ntw_set_frequency_release(float value) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source || !source->audioAnalysis.frequencyRelease) return;
  source->audioAnalysis.frequencyRelease->value = value;
}

void ntw_set_frequency_scale(float value) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source || !source->audioAnalysis.frequencyScale) return;
  source->audioAnalysis.frequencyScale->value = value;
}

void ntw_set_loudness_release(float value) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  source->audioAnalysis.rmsAnalysisParam.release->value = value;
}

void ntw_set_loudness_scale(float value) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  source->audioAnalysis.loudnessScale->value = value;
}

void ntw_select_sample_track(int index) {
  runOnEngineThread([index]() {
    auto svc = AudioSourceService::getService();
    auto& tracks = svc->sampleTracks;
    if (index < 0 || index >= static_cast<int>(tracks.size())) return;
    svc->selectedSampleTrack = tracks[index];
    svc->selectedSampleTrackParam->intValue = index;
    svc->affirmSampleAudioTrack();
  });
}

void ntw_set_file_audio_volume(float volume) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  auto fileSrc = std::dynamic_pointer_cast<FileAudioSource>(source);
  if (fileSrc) {
    fileSrc->setVolume(volume);
  }
}

void ntw_toggle_file_audio_pause(void) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  auto fileSrc = std::dynamic_pointer_cast<FileAudioSource>(source);
  if (!fileSrc) return;
  if (fileSrc->isPaused.load()) {
    fileSrc->resumePlayback();
  } else {
    fileSrc->pausePlayback();
  }
}

void ntw_set_file_audio_position(float position) {
  auto source = AudioSourceService::getService()->selectedAudioSource;
  if (!source) return;
  auto fileSrc = std::dynamic_pointer_cast<FileAudioSource>(source);
  if (fileSrc) {
    fileSrc->setPlaybackPosition(position);
  }
}

// ---------------------------------------------------------------------------
// MARK: - Config / Workspace
// ---------------------------------------------------------------------------

void ntw_save_default_config(void) {
  runOnEngineThread([]() { ConfigService::getService()->saveDefaultConfigFile(); });
}

void ntw_load_default_config(void) {
  runOnEngineThread([]() { ConfigService::getService()->loadDefaultConfigFile(); });
}

void ntw_save_workspace(void) {
  runOnEngineThread([]() { ConfigService::getService()->saveCurrentWorkspace(); });
}

bool ntw_load_workspace(const char* path) {
  if (!path) return false;
  std::string spath(path);
  return runOnEngineThread([spath]() -> bool {
    // Don't call loadWorkspace() — it calls updateWorkspace() →
    // ofSetWindowTitle() (crashes from engine thread).
    //
    // Also can't just call loadConfigFile() directly — if the JSON contains
    // a "workspace" key (e.g. saved by the ImGui app), loadConfigFile will
    // call loadWorkspace() → loadConfigFile() in an infinite recursion.
    //
    // Instead, read the JSON ourselves, strip the workspace key, write it
    // to a temp file, and load that. This avoids both problems.
    std::ifstream fileStream(spath);
    if (!fileStream.is_open()) {
      ofLogError("Bridge") << "ntw_load_workspace: failed to open " << spath;
      return false;
    }

    json data;
    try { fileStream >> data; } catch (...) {
      ofLogError("Bridge") << "ntw_load_workspace: failed to parse JSON";
      return false;
    }
    fileStream.close();
    // Remove workspace key to prevent loadConfigFile → loadWorkspace recursion.
    if (data.contains("workspace")) {
      data.erase("workspace");
    }

    // Write sanitized JSON to a temp file and load it.
    std::string tmpPath = spath + ".ntw_tmp";
    std::ofstream tmpOut(tmpPath);
    if (!tmpOut.is_open()) {
      ofLogError("Bridge") << "ntw_load_workspace: failed to write temp file";
      return false;
    }
    tmpOut << data.dump();
    tmpOut.close();

    ConfigService::getService()->loadConfigFile(tmpPath);
    std::remove(tmpPath.c_str());

    // Extract name from path (filename without extension).
    std::string name = spath;
    auto lastSlash = name.rfind('/');
    if (lastSlash != std::string::npos) name = name.substr(lastSlash + 1);
    auto lastDot = name.rfind('.');
    if (lastDot != std::string::npos) name = name.substr(0, lastDot);

    ConfigService::getService()->currentWorkspace =
      std::make_shared<Workspace>(name, spath);
    return true;
  });
}

void ntw_clear_graph(void) {
  runOnEngineThread([]() {
    VideoSourceService::getService()->clear();
    ShaderChainerService::getService()->clear();
    ConfigService::getService()->closeWorkspace();
  });
  notifyGraphChanged();
}

void ntw_save_workspace_to_path(const char* path, const char* name) {
  if (!path || !name) return;
  std::string spath(path), sname(name);
  runOnEngineThread([spath, sname]() {
    // Don't call saveWorkspace() — it calls updateWorkspace() which calls
    // ofSetWindowTitle(), a UI API that crashes from the engine thread.
    // SwiftUI handles the window title via .navigationTitle() instead.
    //
    // Temporarily clear currentWorkspace before saving so that
    // currentConfig() doesn't embed a self-referencing workspace key
    // in the JSON (which causes loadConfigFile → loadWorkspace →
    // loadConfigFile infinite recursion on load).
    auto prevWorkspace = ConfigService::getService()->currentWorkspace;
    ConfigService::getService()->currentWorkspace = nullptr;
    ConfigService::getService()->saveConfigFile(spath);
    ConfigService::getService()->currentWorkspace =
      std::make_shared<Workspace>(sname, spath);
  });
}

char* ntw_get_current_workspace_path(void) {
  auto ws = ConfigService::getService()->currentWorkspace;
  if (!ws) return nullptr;
  return strdup_safe(ws->path);
}

char* ntw_get_current_workspace_name(void) {
  auto ws = ConfigService::getService()->currentWorkspace;
  if (!ws) return nullptr;
  return strdup_safe(ws->name);
}

// ---------------------------------------------------------------------------
// MARK: - Resolution
// ---------------------------------------------------------------------------

void ntw_set_resolution(int settingIndex) {
  runOnEngineThread([settingIndex]() {
    LayoutStateService::getService()->updateResolutionSettings(settingIndex);
  });
}

int ntw_get_resolution_setting_index(void) {
  return LayoutStateService::getService()->resolutionSetting;
}

void ntw_set_custom_resolution(float width, float height) {
  runOnEngineThread([width, height]() {
    LayoutStateService::getService()->setCustomResolution(width, height);
  });
}

float ntw_get_resolution_width(void) {
  return LayoutStateService::getService()->resolution.x;
}

float ntw_get_resolution_height(void) {
  return LayoutStateService::getService()->resolution.y;
}

// ---------------------------------------------------------------------------
// MARK: - Active Oscillators
// ---------------------------------------------------------------------------

// Helper: find the connectable ID that owns a parameter with the given ownerSettingsId.
static std::string findConnectableIdForSettingsId(const std::string& ownerSettingsId) {
  // Check shaders
  for (auto& shader : ShaderChainerService::getService()->shaders()) {
    auto settings = shader->settingsRef();
    if (settings && settings->settingsId == ownerSettingsId) {
      return shader->connId();
    }
  }
  // Check video sources
  for (auto& source : VideoSourceService::getService()->videoSources()) {
    auto settings = source->settingsRef();
    if (settings && settings->settingsId == ownerSettingsId) {
      return source->connId();
    }
  }
  return "";
}

NTWActiveOscillatorInfo* ntw_get_active_oscillators(int* outCount) {
  if (outCount) *outCount = 0;

  auto activeOscs = OscillationService::getService()->activeOscillators();
  if (activeOscs.empty()) return nullptr;

  // Filter to waveform oscillators only
  std::vector<std::shared_ptr<Oscillator>> waveformOscs;
  for (auto& osc : activeOscs) {
    if (osc->type() == OscillatorType_waveform) {
      waveformOscs.push_back(osc);
    }
  }

  int count = static_cast<int>(waveformOscs.size());
  if (count == 0) return nullptr;
  if (outCount) *outCount = count;

  auto* result = static_cast<NTWActiveOscillatorInfo*>(
    calloc(count, sizeof(NTWActiveOscillatorInfo)));

  for (int i = 0; i < count; i++) {
    auto waveOsc = std::dynamic_pointer_cast<WaveformOscillator>(waveformOscs[i]);
    if (!waveOsc || !waveOsc->value) continue;

    auto& param = waveOsc->value;
    result[i].paramId = strdup_safe(param->paramId);
    result[i].paramName = strdup_safe(param->name);
    result[i].ownerName = strdup_safe(param->ownerName);
    result[i].connectableId = strdup_safe(
      findConnectableIdForSettingsId(param->ownerSettingsId));
    result[i].oscillatorEnabled = waveOsc->enabled->boolValue ? 1 : 0;
    result[i].frequency = waveOsc->frequency ? waveOsc->frequency->value : 0.0f;
    result[i].minOutput = waveOsc->minOutput ? waveOsc->minOutput->value : 0.0f;
    result[i].maxOutput = waveOsc->maxOutput ? waveOsc->maxOutput->value : 1.0f;
    result[i].waveShape = waveOsc->waveShape ? static_cast<int>(waveOsc->waveShape->intValue) : 0;
    result[i].paramMin = param->min;
    result[i].paramMax = param->max;
    result[i].currentValue = param->value;
  }

  return result;
}

void ntw_free_active_oscillator_array(NTWActiveOscillatorInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free(const_cast<char*>(array[i].paramId));
    free(const_cast<char*>(array[i].paramName));
    free(const_cast<char*>(array[i].ownerName));
    free(const_cast<char*>(array[i].connectableId));
  }
  free(array);
}

// ---------------------------------------------------------------------------
// MARK: - Callbacks
// ---------------------------------------------------------------------------

static NTWResolutionChangedCallback resolutionChangedCallback = nullptr;

void ntw_register_graph_changed_callback(NTWGraphChangedCallback callback) {
  graphChangedCallback = callback;
  if (callback) {
    ShaderChainerService::getService()->subscribeToShaderChainerUpdates([callback]() {
      callback();
    });
  }
}

void ntw_register_resolution_changed_callback(NTWResolutionChangedCallback callback) {
  resolutionChangedCallback = callback;
  if (callback) {
    LayoutStateService::getService()->subscribeToResolutionUpdates([callback]() {
      callback();
    });
  }
}

// ---------------------------------------------------------------------------
// MARK: - Preview Manager
// ---------------------------------------------------------------------------

#include "ShaderType.hpp"

struct PreviewInstance {
  std::string previewId;
  std::shared_ptr<Shader> shader;           // The effect shader (null for source previews)
  std::shared_ptr<VideoSource> source;      // The source (shared plasma or standalone)
  bool isSourceOnly = false;                // True if this is a source-only preview
};

static std::unordered_map<std::string, PreviewInstance> previewInstances;
static std::shared_ptr<VideoSource> sharedPlasmaSource;
static int previewCounter = 0;

// Called from IOSurfaceTextureManager.cpp to resolve preview connectable IDs.
// Declared extern there, no lambda/std::function indirection.
std::shared_ptr<Connectable> ntw_find_preview_connectable(const std::string& id) {
  auto it = previewInstances.find(id);
  if (it == previewInstances.end()) return nullptr;
  if (it->second.isSourceOnly && it->second.source) {
    return std::dynamic_pointer_cast<Connectable>(it->second.source);
  }
  if (it->second.shader) {
    return std::dynamic_pointer_cast<Connectable>(it->second.shader);
  }
  return nullptr;
}

char* ntw_create_preview(int shaderTypeRaw, bool isSource) {
  return runOnEngineThread([shaderTypeRaw, isSource]() -> char* {
    std::string pid = "preview_" + std::to_string(++previewCounter);
    PreviewInstance inst;
    inst.previewId = pid;

    if (isSource) {
      auto type = static_cast<ShaderSourceType>(shaderTypeRaw);
      auto src = std::make_shared<ShaderSource>(pid, type);
      src->setup();
      inst.source = std::dynamic_pointer_cast<VideoSource>(src);
      inst.isSourceOnly = true;

      IOSurfaceTextureManager::getInstance()->startSharing(pid);
    } else {
      // Create the effect shader (input frame resolved per-tick from active sources)
      json emptyJson;
      auto shader = ShaderChainerService::getService()->shaderForType(
        static_cast<ShaderType>(shaderTypeRaw), pid, emptyJson);
      if (!shader) return nullptr;
      shader->setup();
      shader->allocateFrames();

      inst.shader = shader;

      IOSurfaceTextureManager::getInstance()->startSharing(pid);
    }

    previewInstances[pid] = inst;
    return strdup_safe(pid);
  });
}

void ntw_destroy_preview(const char* previewId) {
  if (!previewId) return;
  std::string pid(previewId);
  runOnEngineThread([pid]() {
    IOSurfaceTextureManager::getInstance()->stopSharing(pid);
    previewInstances.erase(pid);
  });
}

void ntw_destroy_all_previews(void) {
  runOnEngineThread([]() {
    for (auto& [pid, inst] : previewInstances) {
      IOSurfaceTextureManager::getInstance()->stopSharing(pid);
    }
    previewInstances.clear();
    sharedPlasmaSource.reset();
    previewCounter = 0;
  });
}

// Find the most recently drawn active VideoSource's frame to use as
// input for effect shader previews. Returns nullptr if no sources exist.
static std::shared_ptr<ofFbo> getMostRecentVideoSourceFrame() {
  auto sources = VideoSourceService::getService()->videoSources();
  for (auto it = sources.rbegin(); it != sources.rend(); ++it) {
    if ((*it)->active) {
      auto f = (*it)->frame();
      if (f && f->getWidth() > 0) return f;
    }
  }
  return nullptr;
}

void ntw_tick_previews(void) {
  if (previewInstances.empty()) return;

  auto* mgr = IOSurfaceTextureManager::getInstance();

  // For effect previews: use the most recent active VideoSource's frame,
  // falling back to the shared plasma source if no sources exist on canvas.
  std::shared_ptr<ofFbo> effectInputFrame = getMostRecentVideoSourceFrame();
  if (!effectInputFrame) {
    // No video sources on canvas — fall back to plasma
    if (!sharedPlasmaSource) {
      auto plasma = std::make_shared<ShaderSource>("preview_plasma_shared", ShaderSource_plasma);
      plasma->setup();
      sharedPlasmaSource = std::dynamic_pointer_cast<VideoSource>(plasma);
    }
    if (sharedPlasmaSource) {
      // Render plasma directly, bypassing saveFrame()'s resolution check.
      auto plasmaSrc = std::dynamic_pointer_cast<ShaderSource>(sharedPlasmaSource);
      if (plasmaSrc && plasmaSrc->shader && plasmaSrc->fbo && plasmaSrc->fbo->isAllocated()
          && plasmaSrc->canvas && plasmaSrc->canvas->isAllocated()
          && plasmaSrc->shader->shader.isLoaded()
          && plasmaSrc->fbo->getTexture().getTextureData().textureID != 0
          && plasmaSrc->canvas->getTexture().getTextureData().textureID != 0) {
        plasmaSrc->canvas->begin();
        ofClear(0, 0, 0, 0);
        plasmaSrc->canvas->end();
        plasmaSrc->shader->shade(plasmaSrc->fbo, plasmaSrc->canvas);
        plasmaSrc->fbo->begin();
        ofClear(0, 0, 0, 0);
        plasmaSrc->canvas->draw(0, 0, plasmaSrc->fbo->getWidth(), plasmaSrc->fbo->getHeight());
        plasmaSrc->fbo->end();
      }
      effectInputFrame = sharedPlasmaSource->frame();
    }
  }

  for (auto& [pid, inst] : previewInstances) {
    std::shared_ptr<ofFbo> resultFrame;

    if (inst.isSourceOnly && inst.source) {
      // Render the ShaderSource's internal shader directly, bypassing
      // saveFrame() which checks global resolution and re-allocates FBOs.
      // Preview instances keep their initial resolution — no re-alloc needed.
      auto shaderSrc = std::dynamic_pointer_cast<ShaderSource>(inst.source);
      if (!shaderSrc || !shaderSrc->shader) continue;

      auto& fbo = shaderSrc->fbo;
      auto& canvas = shaderSrc->canvas;
      if (!fbo || !fbo->isAllocated() || !canvas || !canvas->isAllocated()) continue;

      // Verify GL shader program is still valid and textures have valid IDs
      if (!shaderSrc->shader->shader.isLoaded()) continue;
      if (fbo->getTexture().getTextureData().textureID == 0) continue;
      if (canvas->getTexture().getTextureData().textureID == 0) continue;

      canvas->begin();
      ofClear(0, 0, 0, 0);
      canvas->end();

      shaderSrc->shader->shade(fbo, canvas);

      fbo->begin();
      ofClear(0, 0, 0, 0);
      canvas->draw(0, 0, fbo->getWidth(), fbo->getHeight());
      fbo->end();

      resultFrame = fbo;
    } else if (inst.shader) {
      auto lastFrame = inst.shader->lastFrame;
      bool canRender = effectInputFrame && effectInputFrame->isAllocated()
                       && lastFrame && lastFrame->isAllocated()
                       && inst.shader->shader.isLoaded()
                       && lastFrame->getTexture().getTextureData().textureID;

      if (canRender) {
        inst.shader->clearLastFrame();
        // Re-clear to opaque black so unwritten pixels don't show
        // the macOS debug-pink from the transparent clear.
        lastFrame->begin();
        ofClear(0, 0, 0, 255);
        lastFrame->end();
        inst.shader->shade(effectInputFrame, inst.shader->lastFrame);
        resultFrame = inst.shader->frame();
        if (!resultFrame || !resultFrame->isAllocated()) resultFrame = nullptr;
      }

      // If shader couldn't render, clear its frame to black
      if (!resultFrame && lastFrame && lastFrame->isAllocated()
          && lastFrame->getTexture().getTextureData().textureID) {
        lastFrame->begin();
        ofClear(0, 0, 0, 255);
        lastFrame->end();
        resultFrame = lastFrame;
      }
    }

    // Directly blit to IOSurface
    if (resultFrame && resultFrame->getWidth() > 0 && resultFrame->getHeight() > 0) {
      mgr->blitExternalFrame(pid, resultFrame->getId(),
                             resultFrame->getWidth(), resultFrame->getHeight());
    }
  }

  glFlush();
}

// ---------------------------------------------------------------------------
// MARK: - Strand Operations
// ---------------------------------------------------------------------------

bool ntw_save_strand_from_node(const char* nodeId, const char* path, const char* name) {
  if (!nodeId || !path || !name) return false;
  std::string sid(nodeId), spath(path), sname(name);

  return runOnEngineThread([sid, spath, sname]() -> bool {
    auto connectable = findConnectable(sid.c_str());
    if (!connectable) return false;

    auto strand = ShaderChainerService::getService()->strandForConnectable(connectable);
    strand.name = sname;
    ConfigService::getService()->saveStrandFile(strand, spath, "");
    return true;
  });
}

// Strands updated callback
static NTWStrandsUpdatedCallback strandsUpdatedCallback = nullptr;

static void notifyStrandsChanged() {
  if (strandsUpdatedCallback) strandsUpdatedCallback();
}

NTWAvailableStrandInfo* ntw_get_available_strands(int* outCount) {
  if (outCount) *outCount = 0;
  auto strands = StrandService::getService()->availableStrands();
  int count = static_cast<int>(strands.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAvailableStrandInfo*>(
    calloc(count, sizeof(NTWAvailableStrandInfo)));

  for (int i = 0; i < count; i++) {
    result[i].id = strdup_safe(strands[i]->id);
    result[i].name = strdup_safe(strands[i]->name);
    result[i].imagePath = strdup_safe(strands[i]->imagePath);
  }

  return result;
}

void ntw_free_available_strand_info_array(NTWAvailableStrandInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].id);
    free((void*)array[i].name);
    free((void*)array[i].imagePath);
  }
  free(array);
}

char** ntw_load_strand(const char* strandId, int* outCount) {
  if (outCount) *outCount = 0;
  if (!strandId) return nullptr;
  std::string sid(strandId);

  auto ids = runOnEngineThread([sid]() -> std::vector<std::string> {
    auto strand = StrandService::getService()->availableStrandForId(sid);
    if (!strand) return {};
    return ConfigService::getService()->loadStrandFile(strand->path);
  });

  if (ids.empty()) return nullptr;

  int count = static_cast<int>(ids.size());
  if (outCount) *outCount = count;

  char** result = static_cast<char**>(calloc(count, sizeof(char*)));
  for (int i = 0; i < count; i++) {
    result[i] = strdup_safe(ids[i]);
  }

  notifyGraphChanged();
  return result;
}

void ntw_free_string_array(char** array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free(array[i]);
  }
  free(array);
}

NTWAvailableStrandInfo* ntw_get_available_template_strands(int* outCount) {
  if (outCount) *outCount = 0;
  auto strands = StrandService::getService()->availableTemplateStrands();
  int count = static_cast<int>(strands.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWAvailableStrandInfo*>(
    calloc(count, sizeof(NTWAvailableStrandInfo)));

  for (int i = 0; i < count; i++) {
    result[i].id = strdup_safe(strands[i]->id);
    result[i].name = strdup_safe(strands[i]->name);
    result[i].imagePath = strdup_safe(strands[i]->imagePath);
  }

  return result;
}

char** ntw_load_demo_strand(int* outCount) {
  if (outCount) *outCount = 0;

  auto ids = runOnEngineThread([]() -> std::vector<std::string> {
    auto demoStrand = StrandService::getService()->demoStrand;
    if (!demoStrand) return {};
    return ConfigService::getService()->loadStrandFile(demoStrand->path);
  });

  if (ids.empty()) return nullptr;

  int count = static_cast<int>(ids.size());
  if (outCount) *outCount = count;

  char** result = static_cast<char**>(calloc(count, sizeof(char*)));
  for (int i = 0; i < count; i++) {
    result[i] = strdup_safe(ids[i]);
  }

  notifyGraphChanged();
  return result;
}

void ntw_delete_strand(const char* strandId) {
  if (!strandId) return;
  std::string sid(strandId);

  runOnEngineThread([sid]() {
    auto strand = StrandService::getService()->availableStrandForId(sid);
    if (!strand) return;

    // Remove files from disk
    if (!strand->path.empty()) {
      std::filesystem::remove(strand->path);
    }
    if (!strand->imagePath.empty()) {
      std::filesystem::remove(strand->imagePath);
    }

    StrandService::getService()->removeStrand(sid);
  });

  notifyStrandsChanged();
}

void ntw_rename_strand(const char* strandId, const char* newName) {
  if (!strandId || !newName) return;
  std::string sid(strandId), sname(newName);

  runOnEngineThread([sid, sname]() {
    StrandService::getService()->renameStrand(sid, sname);
  });

  notifyStrandsChanged();
}

void ntw_register_strands_updated_callback(NTWStrandsUpdatedCallback callback) {
  strandsUpdatedCallback = callback;
  // Also subscribe to the C++ observable so we fire on internal changes
  StrandService::getService()->subscribeToStrandsUpdated([]() {
    notifyStrandsChanged();
  });
}

// ---------------------------------------------------------------------------
// MARK: - Optional Shaders
// ---------------------------------------------------------------------------

NTWOptionalShaderInfo* ntw_get_optional_shaders(const char* connectableId, int* outCount) {
  if (outCount) *outCount = 0;
  if (!connectableId) return nullptr;

  auto shader = ShaderChainerService::getService()->shaderForId(std::string(connectableId));
  if (!shader) return nullptr;

  auto& optionals = shader->optionalShaders;
  int count = static_cast<int>(optionals.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWOptionalShaderInfo*>(
    calloc(count, sizeof(NTWOptionalShaderInfo)));

  for (int i = 0; i < count; i++) {
    result[i].name = strdup_safe(optionals[i]->name());
    result[i].enabled = optionals[i]->optionallyEnabled;
    result[i].index = i;
  }

  return result;
}

void ntw_free_optional_shader_info_array(NTWOptionalShaderInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].name);
  }
  free(array);
}

void ntw_toggle_optional_shader(const char* connectableId, int index) {
  if (!connectableId) return;
  std::string sid(connectableId);

  runOnEngineThread([sid, index]() {
    auto shader = ShaderChainerService::getService()->shaderForId(sid);
    if (!shader) return;

    auto& optionals = shader->optionalShaders;
    if (index >= 0 && index < static_cast<int>(optionals.size())) {
      optionals[index]->optionallyEnabled = !optionals[index]->optionallyEnabled;
    }
  });
}

NTWParameterInfo* ntw_get_optional_shader_parameters(const char* connectableId, int optionalIndex, int* outCount) {
  if (outCount) *outCount = 0;
  if (!connectableId) return nullptr;

  auto shader = ShaderChainerService::getService()->shaderForId(std::string(connectableId));
  if (!shader) return nullptr;

  auto& optionals = shader->optionalShaders;
  if (optionalIndex < 0 || optionalIndex >= static_cast<int>(optionals.size())) return nullptr;

  auto optShader = optionals[optionalIndex];
  auto settings = optShader->settingsRef();
  if (!settings) return nullptr;

  auto& params = settings->parameters;
  int count = static_cast<int>(params.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWParameterInfo*>(
    calloc(count, sizeof(NTWParameterInfo)));

  for (int i = 0; i < count; i++) {
    auto& p = params[i];
    if (!p) continue;

    result[i].id = strdup_safe(p->paramId);
    result[i].name = strdup_safe(p->name);
    result[i].value = p->value;
    result[i].minValue = p->min;
    result[i].maxValue = p->max;
    result[i].parameterType = static_cast<int>(p->type);
    result[i].intValue = p->intValue;
    result[i].boolValue = p->boolValue ? 1 : 0;
    result[i].isFavorited = p->favorited ? 1 : 0;
    result[i].colorR = p->color->at(0);
    result[i].colorG = p->color->at(1);
    result[i].colorB = p->color->at(2);
    result[i].colorA = p->color->at(3);
    populateOscillatorDriverFields(result[i], p);
  }

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - Screenshot
// ---------------------------------------------------------------------------

void ntw_capture_screenshot(void) {
  runOnEngineThread([]() {
    VideoSourceService::getService()->captureOutputWindowScreenshot();
  });
}

// ---------------------------------------------------------------------------
// MARK: - Library
// ---------------------------------------------------------------------------

static NTWLibraryUpdateCallback libraryUpdateCallback = nullptr;

static void notifyLibraryChanged() {
  if (libraryUpdateCallback) libraryUpdateCallback();
}

void ntw_fetch_library_files(void) {
  // Wrap the fetch so we can notify Swift when it completes.
  std::thread([]() {
    LibraryService::getService()->fetchLibraryFiles();
    notifyLibraryChanged();
  }).detach();
}

int ntw_library_files_loaded(void) {
  return LibraryService::getService()->libraryFiles.empty() ? 0 : 1;
}

NTWLibraryFileInfo* ntw_get_library_files(int* outCount) {
  if (outCount) *outCount = 0;
  auto& files = LibraryService::getService()->libraryFiles;
  int count = static_cast<int>(files.size());
  if (outCount) *outCount = count;
  if (count == 0) return nullptr;

  auto* result = static_cast<NTWLibraryFileInfo*>(
    calloc(count, sizeof(NTWLibraryFileInfo)));

  int i = 0;
  for (auto& [fileId, file] : files) {
    result[i].id = strdup_safe(file->id);
    result[i].name = strdup_safe(file->name);
    result[i].category = strdup_safe(file->category);
    result[i].thumbnailPath = strdup_safe(file->thumbnailPath());
    result[i].isDownloaded = LibraryService::getService()->hasMediaOnDisk(file) ? 1 : 0;
    result[i].isDownloading = file->isDownloading ? 1 : 0;
    result[i].isPaused = file->isPaused ? 1 : 0;
    result[i].downloadProgress = file->progress;
    i++;
  }

  return result;
}

void ntw_free_library_file_info_array(NTWLibraryFileInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].id);
    free((void*)array[i].name);
    free((void*)array[i].category);
    free((void*)array[i].thumbnailPath);
  }
  free(array);
}

char* ntw_add_library_video_source(const char* libraryFileId) {
  if (!libraryFileId) return nullptr;
  std::string fileId(libraryFileId);

  auto result = runOnEngineThread([fileId]() -> char* {
    auto libFile = LibraryService::getService()->libraryFileForId(fileId);
    if (!libFile) return nullptr;
    auto source = ActionService::getService()->addLibraryVideoSource(libFile);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
  notifyGraphChanged();
  return result;
}

void ntw_pause_library_download(const char* libraryFileId) {
  if (!libraryFileId) return;
  auto file = LibraryService::getService()->libraryFileForId(std::string(libraryFileId));
  if (file) {
    LibraryService::getService()->pauseDownload(file);
  }
}

void ntw_resume_library_download(const char* libraryFileId) {
  if (!libraryFileId) return;
  std::string fileId(libraryFileId);
  auto file = LibraryService::getService()->libraryFileForId(fileId);
  if (file) {
    LibraryService::getService()->resumeDownload(file, nullptr);
  }
}

void ntw_register_library_update_callback(NTWLibraryUpdateCallback callback) {
  libraryUpdateCallback = callback;
}

// ---------------------------------------------------------------------------
// MARK: - Library Source State (for download overlay on nodes)
// ---------------------------------------------------------------------------

NTWLibrarySourceState ntw_get_library_source_state(const char* sourceId) {
  NTWLibrarySourceState result = {};
  result.state = 2; // Completed (default for non-library sources)
  result.progress = 0;
  result.isPaused = 0;
  result.libraryFileId = nullptr;

  if (!sourceId) return result;

  auto src = VideoSourceService::getService()->videoSourceForId(std::string(sourceId));
  if (!src || src->type != VideoSource_library) return result;

  auto* libSrc = dynamic_cast<LibrarySource*>(src.get());
  if (!libSrc) return result;

  result.state = static_cast<int>(libSrc->state);
  result.progress = libSrc->libraryFile ? libSrc->libraryFile->progress : 0.0f;
  result.isPaused = (libSrc->libraryFile && libSrc->libraryFile->isPaused) ? 1 : 0;
  result.libraryFileId = libSrc->libraryFile ? strdup_safe(libSrc->libraryFile->id) : nullptr;

  return result;
}

// ---------------------------------------------------------------------------
// MARK: - File Source Playback State
// ---------------------------------------------------------------------------

NTWFileSourceState ntw_get_file_source_state(const char* sourceId) {
  NTWFileSourceState result = {};
  if (!sourceId) return result;

  auto src = VideoSourceService::getService()->videoSourceForId(std::string(sourceId));
  if (!src) return result;

  auto* fileSrc = dynamic_cast<FileSource*>(src.get());
  if (!fileSrc) return result;

  result.isFileSource = 1;
  result.volume = fileSrc->volume ? fileSrc->volume->value : 0.5f;
  result.speed = fileSrc->speed ? fileSrc->speed->value : 1.0f;
  result.position = fileSrc->sliderPosition ? fileSrc->sliderPosition->value : 0.0f;
  result.duration = fileSrc->player.getDuration();
  result.mute = (fileSrc->mute && fileSrc->mute->boolValue) ? 1 : 0;
  result.playing = fileSrc->playing ? 1 : 0;

  return result;
}

void ntw_set_file_source_volume(const char* sourceId, float volume) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src) return;
  auto* fileSrc = dynamic_cast<FileSource*>(src.get());
  if (!fileSrc || !fileSrc->volume) return;
  fileSrc->volume->setValue(volume);
  if (!fileSrc->mute->boolValue) {
    fileSrc->player.setVolume(volume);
  }
}

void ntw_set_file_source_speed(const char* sourceId, float speed) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src) return;
  auto* fileSrc = dynamic_cast<FileSource*>(src.get());
  if (!fileSrc || !fileSrc->speed) return;
  fileSrc->speed->setValue(speed);
}

void ntw_set_file_source_position(const char* sourceId, float position) {
  if (!sourceId) return;
  std::string sid(sourceId);
  runOnEngineThread([sid, position]() {
    auto src = VideoSourceService::getService()->videoSourceForId(sid);
    if (!src) return;
    auto* fileSrc = dynamic_cast<FileSource*>(src.get());
    if (!fileSrc || !fileSrc->sliderPosition) return;
    fileSrc->sliderPosition->setValue(position);
  });
}

void ntw_set_file_source_mute(const char* sourceId, int mute) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src) return;
  auto* fileSrc = dynamic_cast<FileSource*>(src.get());
  if (!fileSrc || !fileSrc->mute) return;
  fileSrc->mute->boolValue = (mute != 0);
  fileSrc->mute->value = mute ? 1.0f : 0.0f;
  fileSrc->player.setVolume(mute ? 0.0f : fileSrc->volume->value);
}

void ntw_set_file_source_playing(const char* sourceId, int playing) {
  if (!sourceId) return;
  std::string sid(sourceId);
  runOnEngineThread([sid, playing]() {
    auto src = VideoSourceService::getService()->videoSourceForId(sid);
    if (!src) return;
    auto* fileSrc = dynamic_cast<FileSource*>(src.get());
    if (!fileSrc) return;
    fileSrc->playing = (playing != 0);
    fileSrc->player.setPaused(!fileSrc->playing);
    if (fileSrc->playing) {
      fileSrc->player.play();
    }
  });
}

// ---------------------------------------------------------------------------
// MARK: - Text Source State
// ---------------------------------------------------------------------------

NTWTextSourceState ntw_get_text_source_state(const char* sourceId) {
  NTWTextSourceState result = {};
  result.text = nullptr;
  result.fontSize = 0;
  result.isTextSource = 0;
  if (!sourceId) return result;

  auto src = VideoSourceService::getService()->videoSourceForId(std::string(sourceId));
  if (!src || src->type != VideoSource_text) return result;

  auto* textSrc = dynamic_cast<TextSource*>(src.get());
  if (!textSrc || !textSrc->displayText) return result;

  result.isTextSource = 1;
  result.text = strdup_safe(textSrc->displayText->text);
  result.fontSize = textSrc->displayText->fontSize;
  result.xPosition = textSrc->displayText->xPosition->value;
  result.yPosition = textSrc->displayText->yPosition->value;
  result.fontIndex = textSrc->displayText->fontSelector->intValue;

  auto& fonts = FontService::getService()->fonts;
  int fontCount = static_cast<int>(fonts.size());
  result.fontCount = fontCount;
  if (fontCount > 0) {
    result.fontNames = static_cast<const char**>(calloc(fontCount, sizeof(const char*)));
    for (int i = 0; i < fontCount; i++) {
      result.fontNames[i] = strdup_safe(fonts[i].name);
    }
  } else {
    result.fontNames = nullptr;
  }

  return result;
}

void ntw_set_text_source_text(const char* sourceId, const char* text) {
  if (!sourceId || !text) return;
  std::string sid(sourceId), stext(text);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src || src->type != VideoSource_text) return;
  auto* textSrc = dynamic_cast<TextSource*>(src.get());
  if (!textSrc || !textSrc->displayText) return;
  textSrc->displayText->text = stext;
}

void ntw_set_text_source_font_size(const char* sourceId, int fontSize) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src || src->type != VideoSource_text) return;
  auto* textSrc = dynamic_cast<TextSource*>(src.get());
  if (!textSrc || !textSrc->displayText) return;
  textSrc->displayText->fontSize = fontSize;
}

void ntw_set_text_source_font_index(const char* sourceId, int fontIndex) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src || src->type != VideoSource_text) return;
  auto* textSrc = dynamic_cast<TextSource*>(src.get());
  if (!textSrc || !textSrc->displayText) return;

  auto& fonts = FontService::getService()->fonts;
  if (fontIndex < 0 || fontIndex >= static_cast<int>(fonts.size())) return;

  textSrc->displayText->fontSelector->intValue = fontIndex;
  textSrc->displayText->fontSelector->affirmIntValue();
  textSrc->displayText->font = fonts[fontIndex];
}

void ntw_set_text_source_position(const char* sourceId, float x, float y) {
  if (!sourceId) return;
  std::string sid(sourceId);
  auto src = VideoSourceService::getService()->videoSourceForId(sid);
  if (!src || src->type != VideoSource_text) return;
  auto* textSrc = dynamic_cast<TextSource*>(src.get());
  if (!textSrc || !textSrc->displayText) return;
  textSrc->displayText->xPosition->value = x;
  textSrc->displayText->yPosition->value = y;
}

// ---------------------------------------------------------------------------
// MARK: - Available Non-Shader Sources
// ---------------------------------------------------------------------------

NTWAvailableNonShaderSourceInfo* ntw_get_available_non_shader_sources(int* outCount) {
  if (outCount) *outCount = 0;

  // Define the non-shader source types we want to expose.
  // VideoSourceType: 0=webcam, 1=file, 2=image, 3=icon, 4=shader, 5=text, 6=typewriter, 7=scrollingText, 8=library, 9=multi, 10=empty, 11=playlist
  struct NonShaderSourceDef {
    VideoSourceType type;
    const char* name;
    const char* icon;
  };

  static const NonShaderSourceDef defs[] = {
    { VideoSource_text,          "Text",           "textformat" },
    { VideoSource_icon,          "Icon",           "face.smiling" },
    { VideoSource_webcam,        "Webcam",         "web.camera" },
  };

  int count = sizeof(defs) / sizeof(defs[0]);
  if (outCount) *outCount = count;

  auto* result = static_cast<NTWAvailableNonShaderSourceInfo*>(
    calloc(count, sizeof(NTWAvailableNonShaderSourceInfo)));

  auto* paramSvc = ParameterService::getService();
  for (int i = 0; i < count; i++) {
    result[i].id = strdup_safe("nonsrc_" + std::to_string(static_cast<int>(defs[i].type)));
    result[i].name = strdup_safe(defs[i].name);
    result[i].icon = strdup_safe(defs[i].icon);
    result[i].sourceType = static_cast<int>(defs[i].type);
    result[i].isFavorited = paramSvc->isFavoriteSourceType(1000 + static_cast<int>(defs[i].type)) ? 1 : 0;
  }

  return result;
}

void ntw_free_available_non_shader_source_info_array(NTWAvailableNonShaderSourceInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].id);
    free((void*)array[i].name);
    free((void*)array[i].icon);
  }
  free(array);
}

// ---------------------------------------------------------------------------
// MARK: - Favorites (Shader Types & Source Types)
// ---------------------------------------------------------------------------

void ntw_toggle_favorite_shader_type(int shaderTypeRaw) {
  ParameterService::getService()->toggleFavoriteShaderType(static_cast<ShaderType>(shaderTypeRaw));
}

int ntw_is_shader_type_favorited(int shaderTypeRaw) {
  return ParameterService::getService()->isShaderTypeFavorited(static_cast<ShaderType>(shaderTypeRaw)) ? 1 : 0;
}

void ntw_toggle_favorite_source_type(int sourceType) {
  ParameterService::getService()->toggleFavoriteSourceType(sourceType);
}

int ntw_is_source_type_favorited(int sourceType) {
  return ParameterService::getService()->isFavoriteSourceType(sourceType) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// MARK: - Strand Sharing & Community
// ---------------------------------------------------------------------------

void ntw_share_strand(const char* nodeId, const char* title, const char* description,
                      const char* author, const void* previewPngData, int previewLen,
                      NTWShareStrandCallback callback) {
  if (!nodeId || !title || !callback) return;
  std::string sid(nodeId), stitle(title);
  std::string sdesc(description ? description : "");
  std::string sauthor(author ? author : "");
  std::string previewBinary;
  if (previewPngData && previewLen > 0) {
    previewBinary.assign(static_cast<const char*>(previewPngData), previewLen);
  }

  // Serialize strand JSON on engine thread
  std::string strandJson = runOnEngineThread([sid]() -> std::string {
    auto connectable = findConnectable(sid.c_str());
    if (!connectable) return "";
    auto strand = ShaderChainerService::getService()->strandForConnectable(connectable);
    return strand.serialize().dump();
  });

  if (strandJson.empty()) {
    callback(false, "Failed to serialize strand");
    return;
  }

  // Upload on detached background thread
  std::thread([=]() {
    httplib::SSLClient cli("nottawa.app");
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(10, 0);
    cli.set_read_timeout(30, 0);
    cli.set_follow_location(true);

    httplib::Headers headers = {{"User-Agent", "nottawa-app/1.0"}};
    httplib::MultipartFormDataItems items;
    items.push_back({"title", stitle, "", "text/plain"});
    items.push_back({"strand", strandJson, "", "application/json"});
    if (!sdesc.empty()) items.push_back({"description", sdesc, "", "text/plain"});
    if (!sauthor.empty()) items.push_back({"author", sauthor, "", "text/plain"});
    if (!previewBinary.empty()) items.push_back({"preview", previewBinary, "preview.png", "image/png"});

    auto res = cli.Post("/api/strands/share", headers, items);

    if (res && res->status == 201) {
      try {
        json responseJson = json::parse(res->body);
        std::string slug;
        if (responseJson.contains("slug")) slug = responseJson["slug"].get<std::string>();
        else if (responseJson.contains("url")) slug = responseJson["url"].get<std::string>();
        callback(true, strdup_safe(slug));
      } catch (...) {
        callback(true, strdup_safe(""));
      }
    } else {
      std::string err = "Upload failed";
      if (res) {
        if (res->status == 429) err = "Too many shares. Please wait.";
        else err = "Server returned status " + std::to_string(res->status);
      } else {
        err = "Network error";
      }
      callback(false, strdup_safe(err));
    }
  }).detach();
}

void ntw_fetch_community_feed(int page, int limit, const char* voterId, NTWFeedCallback callback) {
  if (!callback) return;
  std::string vid(voterId ? voterId : "");

  std::thread([=]() {
    httplib::SSLClient cli("nottawa.app");
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(10, 0);
    cli.set_read_timeout(30, 0);
    cli.set_follow_location(true);

    httplib::Headers headers = {{"User-Agent", "nottawa-app/1.0"}};
    std::string path = "/api/strands/feed?page=" + std::to_string(page) +
                       "&limit=" + std::to_string(limit);
    if (!vid.empty()) path += "&voter_id=" + vid;

    auto res = cli.Get(path.c_str(), headers);

    if (res && res->status == 200) {
      try {
        json responseJson = json::parse(res->body);
        json strandsArr = responseJson.value("strands", json::array());
        int totalPages = 1;
        if (responseJson.contains("pagination") && responseJson["pagination"].is_object()) {
          totalPages = responseJson["pagination"].value("total_pages", 1);
        }
        int count = static_cast<int>(strandsArr.size());

        if (count == 0) {
          callback(true, nullptr, 0, totalPages);
          return;
        }

        // Helper to safely extract a string from JSON (handles null values)
        auto safeStr = [](const json& obj, const char* key) -> std::string {
          if (!obj.contains(key) || obj[key].is_null()) return "";
          return obj[key].get<std::string>();
        };

        auto* result = static_cast<NTWSharedStrandInfo*>(calloc(count, sizeof(NTWSharedStrandInfo)));
        for (int i = 0; i < count; i++) {
          auto& s = strandsArr[i];
          result[i].slug = strdup_safe(safeStr(s, "slug"));
          result[i].title = strdup_safe(safeStr(s, "title"));
          result[i].description = strdup_safe(safeStr(s, "description"));
          result[i].previewUrl = strdup_safe(safeStr(s, "preview_url"));
          result[i].author = strdup_safe(safeStr(s, "author"));
          result[i].upvotes = s.value("upvotes", 0);
          result[i].downvotes = s.value("downvotes", 0);
          result[i].score = s.value("score", 0);
          result[i].views = s.value("views", 0);
          result[i].opens = s.value("opens", 0);
          result[i].createdAt = strdup_safe(safeStr(s, "created_at"));
          result[i].userVote = strdup_safe(safeStr(s, "user_vote"));
        }
        callback(true, result, count, totalPages);
      } catch (...) {
        callback(false, nullptr, 0, 0);
      }
    } else {
      callback(false, nullptr, 0, 0);
    }
  }).detach();
}

void ntw_free_shared_strand_info_array(NTWSharedStrandInfo* array, int count) {
  if (!array) return;
  for (int i = 0; i < count; i++) {
    free((void*)array[i].slug);
    free((void*)array[i].title);
    free((void*)array[i].description);
    free((void*)array[i].previewUrl);
    free((void*)array[i].author);
    free((void*)array[i].createdAt);
    free((void*)array[i].userVote);
  }
  free(array);
}

void ntw_vote_strand(const char* slug, const char* voterId, const char* vote, NTWVoteCallback callback) {
  if (!slug || !voterId || !vote || !callback) return;
  std::string sslug(slug), svid(voterId), svote(vote);

  std::thread([=]() {
    httplib::SSLClient cli("nottawa.app");
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(10, 0);
    cli.set_read_timeout(15, 0);
    cli.set_follow_location(true);

    httplib::Headers headers = {
      {"User-Agent", "nottawa-app/1.0"},
      {"Content-Type", "application/json"}
    };

    json body;
    body["voter_id"] = svid;
    body["vote"] = svote;

    std::string path = "/api/strands/" + sslug + "/vote";
    auto res = cli.Post(path.c_str(), headers, body.dump(), "application/json");

    if (res && res->status == 200) {
      try {
        json r = json::parse(res->body);
        std::string uv;
        if (r.contains("user_vote") && !r["user_vote"].is_null())
          uv = r["user_vote"].get<std::string>();
        callback(true,
                 r.value("upvotes", 0),
                 r.value("downvotes", 0),
                 r.value("score", 0),
                 strdup_safe(uv));
      } catch (...) {
        callback(false, 0, 0, 0, strdup_safe(""));
      }
    } else {
      callback(false, 0, 0, 0, strdup_safe(""));
    }
  }).detach();
}

void ntw_fetch_shared_strand(const char* slug, NTWFetchStrandCallback callback) {
  if (!slug || !callback) return;
  std::string sslug(slug);

  std::thread([=]() {
    httplib::SSLClient cli("nottawa.app");
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(15, 0);
    cli.set_read_timeout(30, 0);
    cli.set_follow_location(true);

    httplib::Headers headers = {{"User-Agent", "nottawa-app/1.0"}};
    std::string path = "/api/strands/shared/" + sslug;
    auto res = cli.Get(path.c_str(), headers);

    if (res && res->status == 200) {
      try {
        json responseJson = json::parse(res->body);
        std::string strandJson = responseJson.value("strand", "");
        callback(true, strdup_safe(strandJson));
      } catch (...) {
        callback(false, strdup_safe("Failed to parse strand data"));
      }
    } else {
      std::string err = "Fetch failed";
      if (res) {
        if (res->status == 404) err = "Strand not found";
        else err = "Server returned status " + std::to_string(res->status);
      } else {
        err = "Network error";
      }
      callback(false, strdup_safe(err));
    }
  }).detach();
}

char** ntw_load_strand_from_json(const char* jsonStr, int* outCount) {
  if (outCount) *outCount = 0;
  if (!jsonStr) return nullptr;
  std::string sjson(jsonStr);

  auto ids = runOnEngineThread([sjson]() -> std::vector<std::string> {
    return ConfigService::getService()->loadStrandJson(sjson);
  });

  if (ids.empty()) return nullptr;

  int count = static_cast<int>(ids.size());
  if (outCount) *outCount = count;

  char** result = static_cast<char**>(calloc(count, sizeof(char*)));
  for (int i = 0; i < count; i++) {
    result[i] = strdup_safe(ids[i]);
  }

  notifyGraphChanged();
  return result;
}

char* ntw_get_strand_json_for_node(const char* nodeId) {
  if (!nodeId) return nullptr;
  std::string sid(nodeId);

  return runOnEngineThread([sid]() -> char* {
    auto connectable = findConnectable(sid.c_str());
    if (!connectable) return nullptr;
    auto strand = ShaderChainerService::getService()->strandForConnectable(connectable);
    return strdup_safe(strand.serialize().dump());
  });
}

// ---------------------------------------------------------------------------
// MARK: - Texture Sharing (IOSurface)
// ---------------------------------------------------------------------------

void ntw_start_texture_sharing(const char* connectableId) {
  if (!connectableId) return;
  IOSurfaceTextureManager::getInstance()->startSharing(std::string(connectableId));
}

void ntw_stop_texture_sharing(const char* connectableId) {
  if (!connectableId) return;
  IOSurfaceTextureManager::getInstance()->stopSharing(std::string(connectableId));
}

void* ntw_get_connectable_texture(const char* connectableId) {
  if (!connectableId) return nullptr;
  return IOSurfaceTextureManager::getInstance()->getIOSurface(std::string(connectableId));
}

int ntw_get_texture_width(const char* connectableId) {
  if (!connectableId) return 0;
  return IOSurfaceTextureManager::getInstance()->getWidth(std::string(connectableId));
}

int ntw_get_texture_height(const char* connectableId) {
  if (!connectableId) return 0;
  return IOSurfaceTextureManager::getInstance()->getHeight(std::string(connectableId));
}
