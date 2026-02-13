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
#include "Workspace.hpp"
#include "LibraryService.hpp"
#include "LibraryFile.hpp"
#include "FileSource.hpp"
#include "ofMain.h"

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
  return runOnEngineThread([sname]() -> char* {
    auto source = ActionService::getService()->addTextVideoSource(sname);
    if (!source) return nullptr;
    return strdup_safe(source->id);
  });
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

  for (int i = 0; i < count; i++) {
    auto& as = filtered[i];
    result[i].id = strdup_safe(std::to_string(static_cast<int>(as->type)));
    result[i].name = strdup_safe(as->name);
    result[i].category = nullptr;
    result[i].shaderTypeRaw = static_cast<int>(as->type);
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

  for (int i = 0; i < count; i++) {
    auto type = AvailableShaderSourceTypes[i];
    std::string name = shaderSourceTypeName(type);
    std::string cat = shaderSourceTypeCategory(type);
    result[i].id = strdup_safe("src_" + std::to_string(static_cast<int>(type)));
    result[i].name = strdup_safe(name);
    result[i].category = strdup_safe(cat);
    result[i].shaderTypeRaw = static_cast<int>(type);
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
    result[i].sourceType = 0; // Default to mic; subtypes determined by dynamic_cast if needed
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
    } else if (inst.shader && effectInputFrame) {
      if (!effectInputFrame->isAllocated()) continue;
      auto lastFrame = inst.shader->lastFrame;
      if (!lastFrame || !lastFrame->isAllocated()) continue;

      // Verify GL shader program is still valid
      if (!inst.shader->shader.isLoaded()) continue;
      if (!lastFrame->getTexture().getTextureData().textureID) continue;

      inst.shader->clearLastFrame();
      inst.shader->shade(effectInputFrame, inst.shader->lastFrame);
      resultFrame = inst.shader->frame();
      if (!resultFrame || !resultFrame->isAllocated()) continue;
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
