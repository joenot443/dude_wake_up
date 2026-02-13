//
//  nottawa_bridge.h
//  dude_wake_up
//
//  C bridging API between the C++ engine and the SwiftUI frontend.
//  All functions use plain C types only. String ownership is documented
//  per-function; callers must free strings returned by ntw_* functions
//  unless stated otherwise.
//

#ifndef nottawa_bridge_h
#define nottawa_bridge_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// MARK: - Data Structs
// ---------------------------------------------------------------------------

typedef struct {
  const char* id;          // Owned by caller (strdup'd) - must free
  const char* name;        // Owned by caller (strdup'd) - must free
  float value;
  float minValue;
  float maxValue;
  int parameterType;       // 0=Standard, 1=Int, 2=Bool, 3=Color, 4=Hidden
  int intValue;
  int boolValue;
  int isFavorited;
  float colorR, colorG, colorB, colorA;

  // Oscillator info (populated if parameter has a paired WaveformOscillator)
  int hasOscillator;           // 1 if parameter has a paired WaveformOscillator
  int oscillatorEnabled;       // osc->enabled->boolValue
  float oscillatorFrequency;   // 0.0-4.0
  float oscillatorMinOutput;
  float oscillatorMaxOutput;
  int oscillatorWaveShape;     // 0-10 (WaveShape enum)

  // Audio driver info
  int hasDriver;               // 1 if parameter->driver != nullptr
  const char* driverName;      // Owned by caller - must free. NULL if no driver
  float driverShift;           // -1.0 to 1.0
  float driverScale;           // 0.0 to 2.0
} NTWParameterInfo;

typedef struct {
  const char* id;          // Owned by caller - must free
  const char* name;        // Owned by caller - must free
  int shaderTypeRaw;       // ShaderType enum value
  int connectableType;     // 0=Source, 1=Shader
  int inputCount;
  float posX, posY;
} NTWConnectableInfo;

typedef struct {
  const char* id;          // Owned by caller - must free
  const char* startId;     // Owned by caller - must free
  const char* endId;       // Owned by caller - must free
  int connectionType;      // 0=Shader, 1=Source
  int inputSlot;
  int outputSlot;
} NTWConnectionInfo;

typedef struct {
  const char* id;          // Owned by caller - must free
  const char* name;        // Owned by caller - must free
  const char* category;    // Owned by caller - must free (may be NULL for shaders)
  int shaderTypeRaw;       // ShaderType enum value
} NTWAvailableShaderInfo;

typedef struct {
  const char* id;          // Owned by caller - must free
  const char* name;        // Owned by caller - must free
  int sourceType;          // VideoSourceType enum value
  int inputCount;
  float posX, posY;
} NTWVideoSourceInfo;

typedef struct {
  const char* id;          // Owned by caller - must free
  const char* name;        // Owned by caller - must free
  int sourceType;          // AudioSourceType (0=Mic, 1=File, 2=System)
} NTWAudioSourceInfo;

typedef struct {
  float rms;
  float lows, mids, highs;
  float bpm;
  float beatPulse;
  bool beatDetected;
} NTWAudioAnalysisInfo;

// ---------------------------------------------------------------------------
// MARK: - Memory Management
// ---------------------------------------------------------------------------

// Free a single NTWParameterInfo's strings
void ntw_free_parameter_info(NTWParameterInfo* info);

// Free an array of NTWParameterInfo (frees each element's strings + the array)
void ntw_free_parameter_info_array(NTWParameterInfo* array, int count);

// Free a single NTWConnectableInfo's strings
void ntw_free_connectable_info(NTWConnectableInfo* info);

// Free an array of NTWConnectableInfo
void ntw_free_connectable_info_array(NTWConnectableInfo* array, int count);

// Free a single NTWConnectionInfo's strings
void ntw_free_connection_info(NTWConnectionInfo* info);

// Free an array of NTWConnectionInfo
void ntw_free_connection_info_array(NTWConnectionInfo* array, int count);

// Free a single NTWAvailableShaderInfo's strings
void ntw_free_available_shader_info(NTWAvailableShaderInfo* info);

// Free an array of NTWAvailableShaderInfo
void ntw_free_available_shader_info_array(NTWAvailableShaderInfo* array, int count);

// Free a single NTWVideoSourceInfo's strings
void ntw_free_video_source_info(NTWVideoSourceInfo* info);

// Free an array of NTWVideoSourceInfo
void ntw_free_video_source_info_array(NTWVideoSourceInfo* array, int count);

// Free a single NTWAudioSourceInfo's strings
void ntw_free_audio_source_info(NTWAudioSourceInfo* info);

// Free an array of NTWAudioSourceInfo
void ntw_free_audio_source_info_array(NTWAudioSourceInfo* array, int count);

// Free a string returned by ntw_* functions
void ntw_free_string(char* str);

// ---------------------------------------------------------------------------
// MARK: - Engine Lifecycle
// ---------------------------------------------------------------------------

// Initialize the engine: create OpenGL context, set up services.
// Returns true on success.
bool ntw_engine_init(void);

// Start the engine tick loop (runs on a background thread at ~60fps).
void ntw_engine_start(void);

// Stop the engine tick loop.
void ntw_engine_stop(void);

// Perform a single engine tick (for manual stepping / testing).
void ntw_engine_tick(void);

// Returns true if the engine has been initialized.
bool ntw_engine_is_initialized(void);

// Returns the engine's current ticks-per-second.
float ntw_engine_fps(void);

// ---------------------------------------------------------------------------
// MARK: - Shader Mutations (via ActionService)
// ---------------------------------------------------------------------------

// Add a shader of the given type. Returns the new shader's ID (caller must free).
char* ntw_add_shader(int shaderTypeRaw);

// Remove a shader by its ID.
void ntw_remove_shader(const char* shaderId);

// Create a blend shader connecting two existing connectables.
// Returns the new blend shader's ID (caller must free), or NULL on failure.
char* ntw_create_blend_between(const char* id1, const char* id2);

// Replace a shader's type in-place, preserving connections.
// Returns the new shader's ID (caller must free), or NULL on failure.
char* ntw_replace_shader(const char* shaderId, int newShaderTypeRaw);

// Get the fragment shader file path for a shader.
// Returns the path (caller must free), or NULL if not found.
char* ntw_get_shader_file_path(const char* shaderId);

// Check if a shader supports auxiliary output.
bool ntw_supports_aux_output(const char* shaderId);

// ---------------------------------------------------------------------------
// MARK: - Video Source Mutations (via ActionService)
// ---------------------------------------------------------------------------

// Each returns the new source's ID (caller must free).
char* ntw_add_shader_video_source(int shaderSourceTypeRaw);
char* ntw_add_text_video_source(const char* name);
char* ntw_add_image_video_source(const char* name, const char* path);
char* ntw_add_file_video_source(const char* name, const char* path);
char* ntw_add_webcam_video_source(const char* name, int deviceId);
char* ntw_add_icon_video_source(const char* name);
char* ntw_add_playlist_video_source(const char* name);

// Remove a video source by its ID.
void ntw_remove_video_source(const char* sourceId);

// Replace a shader video source's type in-place, preserving connections.
// Returns the source ID (caller must free), or NULL on failure.
char* ntw_replace_video_source(const char* videoSourceId, int shaderSourceTypeRaw);

// ---------------------------------------------------------------------------
// MARK: - Connection Mutations (via ActionService)
// ---------------------------------------------------------------------------

// Create a connection. Returns the new connection's ID (caller must free),
// or NULL on failure.
char* ntw_make_connection(const char* startId,
                          const char* endId,
                          int connectionType,
                          int outputSlot,
                          int inputSlot);

// Remove a connection by its ID.
void ntw_remove_connection(const char* connectionId);

// ---------------------------------------------------------------------------
// MARK: - Undo / Redo / Copy / Paste
// ---------------------------------------------------------------------------

void ntw_undo(void);
void ntw_redo(void);
bool ntw_can_undo(void);
bool ntw_can_redo(void);

// ---------------------------------------------------------------------------
// MARK: - Queries: Shaders
// ---------------------------------------------------------------------------

// Get all active shaders. Caller owns the returned array and must free it
// with ntw_free_connectable_info_array().
NTWConnectableInfo* ntw_get_all_shaders(int* outCount);

// Get info for a single shader. Returns zeroed struct if not found.
NTWConnectableInfo ntw_get_shader(const char* shaderId);

// ---------------------------------------------------------------------------
// MARK: - Queries: Video Sources
// ---------------------------------------------------------------------------

// Get all active video sources. Caller owns the returned array.
NTWVideoSourceInfo* ntw_get_all_video_sources(int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Queries: Connections
// ---------------------------------------------------------------------------

// Get all connections. Caller owns the returned array.
NTWConnectionInfo* ntw_get_all_connections(int* outCount);

// Get all connections involving a specific connectable (as start or end).
// Caller owns the returned array.
NTWConnectionInfo* ntw_get_connections_for_connectable(const char* connectableId, int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Queries: Parameters
// ---------------------------------------------------------------------------

// Get all parameters for a connectable (shader or source).
// Caller owns the returned array.
NTWParameterInfo* ntw_get_parameters(const char* connectableId, int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Parameter Control
// ---------------------------------------------------------------------------

void ntw_set_parameter_value(const char* paramId, float value);
void ntw_set_parameter_int(const char* paramId, int value);
void ntw_set_parameter_bool(const char* paramId, bool value);
void ntw_set_parameter_color(const char* paramId,
                             float r, float g, float b, float a);
void ntw_toggle_parameter_favorite(const char* paramId);

// ---------------------------------------------------------------------------
// MARK: - Oscillator Control
// ---------------------------------------------------------------------------

void ntw_set_oscillator_enabled(const char* paramId, bool enabled);
void ntw_set_oscillator_frequency(const char* paramId, float frequency);
void ntw_set_oscillator_min_output(const char* paramId, float minOutput);
void ntw_set_oscillator_max_output(const char* paramId, float maxOutput);
void ntw_set_oscillator_wave_shape(const char* paramId, int waveShape);

// ---------------------------------------------------------------------------
// MARK: - Audio Driver Control
// ---------------------------------------------------------------------------

void ntw_set_parameter_driver(const char* paramId, const char* audioParamName);
void ntw_remove_parameter_driver(const char* paramId);
void ntw_set_parameter_driver_shift(const char* paramId, float shift);
void ntw_set_parameter_driver_scale(const char* paramId, float scale);

typedef struct {
    const char* name;   // Owned by caller - must free
    int index;
} NTWAudioDriverParam;

NTWAudioDriverParam* ntw_get_available_audio_drivers(int* outCount);
void ntw_free_audio_driver_param_array(NTWAudioDriverParam* array, int count);

// ---------------------------------------------------------------------------
// MARK: - Queries: Available Shaders (for browser UI)
// ---------------------------------------------------------------------------

// Get available shaders for a category.
// category: 0=All, 1=Basic, 2=Mix, 3=Transform, 4=Filter, 5=Mask, 6=Glitch
NTWAvailableShaderInfo* ntw_get_available_shaders(int category, int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Node Position & Active State
// ---------------------------------------------------------------------------

// Set the canvas position for a connectable (shader or video source).
void ntw_set_connectable_position(const char* connectableId, float x, float y);

// Set/get whether a connectable is active (processing frames).
void ntw_set_connectable_active(const char* connectableId, bool active);
bool ntw_get_connectable_active(const char* connectableId);

// Remove a connectable by ID (works for both shaders and video sources).
void ntw_remove_connectable(const char* connectableId);

// ---------------------------------------------------------------------------
// MARK: - Queries: Available Video Sources (for browser UI)
// ---------------------------------------------------------------------------

// Get available shader source types for the video source browser.
// Returns an array of NTWAvailableShaderInfo (name + shaderSourceType raw value).
NTWAvailableShaderInfo* ntw_get_available_shader_sources(int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Selection
// ---------------------------------------------------------------------------

void ntw_select_connectable(const char* connectableId);
void ntw_deselect_connectable(void);

// Returns the selected connectable's ID, or NULL if nothing selected.
// Caller must free.
char* ntw_get_selected_connectable_id(void);

// ---------------------------------------------------------------------------
// MARK: - Audio
// ---------------------------------------------------------------------------

NTWAudioAnalysisInfo ntw_get_audio_analysis(void);
float ntw_get_current_bpm(void);

// Get all audio sources. Caller owns the returned array.
NTWAudioSourceInfo* ntw_get_all_audio_sources(int* outCount);

// Select an audio source by ID.
void ntw_select_audio_source(const char* audioSourceId);

// ---------------------------------------------------------------------------
// MARK: - Config / Workspace
// ---------------------------------------------------------------------------

void ntw_save_default_config(void);
void ntw_load_default_config(void);

// Save/load workspace (uses file dialogs or default paths).
void ntw_save_workspace(void);
bool ntw_load_workspace(const char* path);

// Clear the entire graph (all shaders, sources, connections).
void ntw_clear_graph(void);

// Save full workspace config to a specific file path.
// Sets this path as the "current workspace" for future saves.
void ntw_save_workspace_to_path(const char* path, const char* name);

// Get the current workspace path, or NULL if no workspace is open.
// Caller must free the returned string.
char* ntw_get_current_workspace_path(void);

// Get the current workspace name, or NULL if no workspace is open.
// Caller must free the returned string.
char* ntw_get_current_workspace_name(void);

// ---------------------------------------------------------------------------
// MARK: - Resolution
// ---------------------------------------------------------------------------

void ntw_set_resolution(int settingIndex);
void ntw_set_custom_resolution(float width, float height);
float ntw_get_resolution_width(void);
float ntw_get_resolution_height(void);

// ---------------------------------------------------------------------------
// MARK: - Preview Manager (for browser live previews)
// ---------------------------------------------------------------------------

// Create a preview instance for a shader type. A shared PlasmaShaderSource feeds
// into a new shader of the given type, and its output is shared via IOSurface.
// Returns a preview ID (caller must free) that can be used with
// ntw_get_connectable_texture() to read the IOSurface.
// isSource: if true, creates a ShaderSource preview (no input needed).
char* ntw_create_preview(int shaderTypeRaw, bool isSource);

// Destroy a single preview instance.
void ntw_destroy_preview(const char* previewId);

// Destroy all preview instances (call when browser closes).
void ntw_destroy_all_previews(void);

// Tick all preview instances (called from engine loop).
void ntw_tick_previews(void);

// ---------------------------------------------------------------------------
// MARK: - Strand Operations
// ---------------------------------------------------------------------------

// Save a strand from a node to a file.
// Returns true on success.
bool ntw_save_strand_from_node(const char* nodeId, const char* path, const char* name);

// ---------------------------------------------------------------------------
// MARK: - Optional Shaders
// ---------------------------------------------------------------------------

typedef struct {
  const char* name;      // Owned by caller - must free
  bool enabled;
  int index;
} NTWOptionalShaderInfo;

// Get optional shaders for a connectable.
NTWOptionalShaderInfo* ntw_get_optional_shaders(const char* connectableId, int* outCount);

// Free an array of NTWOptionalShaderInfo.
void ntw_free_optional_shader_info_array(NTWOptionalShaderInfo* array, int count);

// Toggle an optional shader on/off.
void ntw_toggle_optional_shader(const char* connectableId, int index);

// Get parameters for an optional shader.
NTWParameterInfo* ntw_get_optional_shader_parameters(const char* connectableId, int optionalIndex, int* outCount);

// ---------------------------------------------------------------------------
// MARK: - Active Oscillators
// ---------------------------------------------------------------------------

typedef struct {
    const char* paramId;       // Parameter ID (key for mutations via ntw_set_oscillator_* functions) - must free
    const char* paramName;     // e.g. "Speed" - must free
    const char* ownerName;     // e.g. "Plasma" (from Parameter::ownerName) - must free
    const char* connectableId; // Shader/source ID - must free
    int oscillatorEnabled;     // 1 if enabled
    float frequency;
    float minOutput;
    float maxOutput;
    int waveShape;             // 0-10 (WaveShape enum)
    float paramMin;            // Parameter's min bound
    float paramMax;            // Parameter's max bound
    float currentValue;        // Current parameter value
} NTWActiveOscillatorInfo;

// Get all active (enabled) waveform oscillators.
// Caller owns the returned array and must free with ntw_free_active_oscillator_array().
NTWActiveOscillatorInfo* ntw_get_active_oscillators(int* outCount);

// Free an array of NTWActiveOscillatorInfo.
void ntw_free_active_oscillator_array(NTWActiveOscillatorInfo* array, int count);

// ---------------------------------------------------------------------------
// MARK: - Callbacks
// ---------------------------------------------------------------------------

// Register a callback that fires when the shader graph changes
// (shaders added/removed, connections changed).
typedef void (*NTWGraphChangedCallback)(void);
void ntw_register_graph_changed_callback(NTWGraphChangedCallback callback);

// Register a callback that fires when the resolution changes.
typedef void (*NTWResolutionChangedCallback)(void);
void ntw_register_resolution_changed_callback(NTWResolutionChangedCallback callback);

// ---------------------------------------------------------------------------
// MARK: - Screenshot
// ---------------------------------------------------------------------------

// Capture a screenshot of the output window.
void ntw_capture_screenshot(void);

// ---------------------------------------------------------------------------
// MARK: - Library
// ---------------------------------------------------------------------------

typedef struct {
    const char* id;              // Owned by caller (strdup'd) - must free
    const char* name;            // Owned by caller (strdup'd) - must free
    const char* category;        // Owned by caller (strdup'd) - must free
    const char* thumbnailPath;   // Owned by caller (strdup'd) - must free (empty if no thumbnail)
    int isDownloaded;            // 1 if video exists on disk
    int isDownloading;           // 1 if currently downloading
    int isPaused;                // 1 if download paused
    float downloadProgress;      // 0.0-1.0
} NTWLibraryFileInfo;

// Trigger background fetch of library files from server.
void ntw_fetch_library_files(void);

// Returns 1 if library files have been fetched (non-empty).
int ntw_library_files_loaded(void);

// Get all fetched library files. Caller owns the returned array.
NTWLibraryFileInfo* ntw_get_library_files(int* outCount);

// Free an array of NTWLibraryFileInfo.
void ntw_free_library_file_info_array(NTWLibraryFileInfo* array, int count);

// Add a LibrarySource video source. Returns the new source ID (caller must free),
// or NULL on failure.
char* ntw_add_library_video_source(const char* libraryFileId);

// Pause/resume a library file download.
void ntw_pause_library_download(const char* libraryFileId);
void ntw_resume_library_download(const char* libraryFileId);

// Register a callback that fires when library data changes
// (fetch complete, thumbnails ready, download progress).
typedef void (*NTWLibraryUpdateCallback)(void);
void ntw_register_library_update_callback(NTWLibraryUpdateCallback callback);

// ---------------------------------------------------------------------------
// MARK: - Library Source State (for download overlay on nodes)
// ---------------------------------------------------------------------------

typedef struct {
    int state;              // LibrarySourceState enum (0=Waiting, 1=Downloading, 2=Completed, 3=Failed)
    float progress;         // 0.0-1.0
    int isPaused;           // 1 if download paused
    const char* libraryFileId;  // Owned by caller (strdup'd) - must free. NULL if not a library source.
} NTWLibrarySourceState;

// Get the download state for a video source node.
// Returns state=2 (Completed) with progress=0 if the source is not a LibrarySource.
NTWLibrarySourceState ntw_get_library_source_state(const char* sourceId);

// ---------------------------------------------------------------------------
// MARK: - File Source Playback State
// ---------------------------------------------------------------------------

typedef struct {
    float volume;       // 0.0–1.0
    float speed;        // 0.0–2.0
    float position;     // 0.0–1.0 (playback position)
    float duration;     // total duration in seconds
    int mute;           // 1 if muted
    int playing;        // 1 if playing
    int isFileSource;   // 1 if this connectable is a FileSource (or subclass)
} NTWFileSourceState;

NTWFileSourceState ntw_get_file_source_state(const char* sourceId);
void ntw_set_file_source_volume(const char* sourceId, float volume);
void ntw_set_file_source_speed(const char* sourceId, float speed);
void ntw_set_file_source_position(const char* sourceId, float position);
void ntw_set_file_source_mute(const char* sourceId, int mute);
void ntw_set_file_source_playing(const char* sourceId, int playing);

// ---------------------------------------------------------------------------
// MARK: - Texture Sharing (IOSurface)
// ---------------------------------------------------------------------------

// Start sharing the output texture of a connectable via IOSurface.
// GL resources are created lazily on the next engine tick.
void ntw_start_texture_sharing(const char* connectableId);

// Stop sharing and release IOSurface resources.
void ntw_stop_texture_sharing(const char* connectableId);

// Get the IOSurface for a connectable's output texture.
// Returns the IOSurfaceRef as void* (caller does NOT own it; do not CFRelease).
// Returns NULL if not sharing or not yet set up.
void* ntw_get_connectable_texture(const char* connectableId);

// Get the current texture dimensions for a shared connectable.
int ntw_get_texture_width(const char* connectableId);
int ntw_get_texture_height(const char* connectableId);

#ifdef __cplusplus
}
#endif

#endif /* nottawa_bridge_h */
