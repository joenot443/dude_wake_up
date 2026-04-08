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

  // MIDI binding info
  int hasMidiBinding;          // 1 if parameter has a MIDI pairing
  const char* midiDescriptor;  // Owned by caller - must free. NULL if no binding

  // Options (for int params with named choices, e.g. font selector)
  const char** options;        // Array of option strings (NULL if none) - caller must free each + array
  int optionCount;             // 0 if no options
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
  int isFavorited;         // 1 if this shader/source type is favorited
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
bool ntw_is_audio_reactive(const char* connectableId);
bool ntw_is_audio_active(void);

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

// Copy selected connectables to internal clipboard.
void ntw_copy_connectables(const char** ids, int count);

// Paste copied connectables. Returns new IDs (caller must free with ntw_free_string_array).
char** ntw_paste_connectables(int* outCount);

// Returns true if the internal clipboard has connectables.
bool ntw_has_copied_connectables(void);

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
void ntw_reset_parameter(const char* paramId);
void ntw_reset_all_parameters(const char* connectableId);
void ntw_clear_feedback_buffer(const char* shaderId);

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

// Set/get whether a shader is bypassed (passes input through without processing).
void ntw_set_shader_bypassed(const char* shaderId, int bypassed);
int ntw_get_shader_bypassed(const char* shaderId);

// Get the depth of a shader in its chain (0 = directly connected to source).
int ntw_get_shader_depth(const char* shaderId);

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
// MARK: - Extended Audio (for Audio Panel)
// ---------------------------------------------------------------------------

typedef struct {
  // Core analysis values
  float rms;
  float lows, mids, highs;
  float bpm;
  float beatPulse;

  // BPM settings
  int bpmMode;          // 0=Auto, 1=Manual, 2=Link
  bool bpmLocked;
  float bpmNudge;       // -1.0 to 1.0
  bool bpmEnabled;

  // Frequency settings
  int smoothingMode;    // 0=Exp, 1=MovAvg, 2=None, 3=PeakHold
  float frequencyRelease;
  float frequencyScale;
  float loudnessRelease;
  float loudnessScale;

  // Beat detection
  uint32_t beatCount;   // Increments on each detected beat

  // State
  bool audioActive;
  int audioSourceType;  // 0=Mic, 1=File, 2=System

  // Spectrum data (fixed-size for C interop)
  float melSpectrum[13];
  int melSpectrumCount;

  // Waveform data (downsampled)
  float waveform[256];
  int waveformCount;
} NTWExtendedAudioAnalysisInfo;

typedef struct {
  float volume;
  bool isPaused;
  float playbackPosition;  // 0.0-1.0
  float totalDuration;     // seconds
  bool isFileSource;
  int selectedTrackIndex;
} NTWFileAudioState;

typedef struct {
  const char* name;   // Owned by caller - must free
  const char* path;   // Owned by caller - must free
  int bpm;
  int index;
} NTWAudioTrackInfo;

// Extended audio analysis snapshot (primary 30fps polling call)
NTWExtendedAudioAnalysisInfo ntw_get_extended_audio_analysis(void);

// Sample tracks
NTWAudioTrackInfo* ntw_get_sample_tracks(int* outCount);
void ntw_free_audio_track_info_array(NTWAudioTrackInfo* array, int count);

// File audio state
NTWFileAudioState ntw_get_file_audio_state(void);

// Audio source ID
const char* ntw_get_selected_audio_source_id(void);

// Audio mutations (dispatched to engine thread)
void ntw_toggle_audio_source(void);
void ntw_set_bpm_mode(int mode);
void ntw_set_bpm_locked(bool locked);
void ntw_set_bpm_value(float bpm);
void ntw_nudge_bpm(float delta);
void ntw_set_bpm_nudge(float value);
void ntw_tap_bpm(void);
void ntw_set_bpm_enabled(bool enabled);
void ntw_set_smoothing_mode(int mode);
void ntw_set_frequency_release(float value);
void ntw_set_frequency_scale(float value);
void ntw_set_loudness_release(float value);
void ntw_set_loudness_scale(float value);
void ntw_select_sample_track(int index);
void ntw_set_file_audio_volume(float volume);
void ntw_toggle_file_audio_pause(void);
void ntw_set_file_audio_position(float position);

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
int ntw_get_resolution_setting_index(void);
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

// Set the preferred video source ID for effect preview input.
// Pass NULL to clear the preference (falls back to any active source).
void ntw_set_preferred_preview_source(const char* sourceId);

// ---------------------------------------------------------------------------
// MARK: - Strand Operations
// ---------------------------------------------------------------------------

// Get the strands folder path. Caller must free the returned string.
char* ntw_get_strands_folder_path(void);

// Save a strand from a node to a file.
// Returns true on success.
bool ntw_save_strand_from_node(const char* nodeId, const char* path, const char* name);

typedef struct {
    const char* id;        // Owned by caller (strdup'd) - must free
    const char* name;      // Owned by caller (strdup'd) - must free
    const char* imagePath; // Owned by caller (strdup'd) - must free (empty if no preview)
} NTWAvailableStrandInfo;

// Get all available strands. Caller owns the returned array.
NTWAvailableStrandInfo* ntw_get_available_strands(int* outCount);

// Free an array of NTWAvailableStrandInfo.
void ntw_free_available_strand_info_array(NTWAvailableStrandInfo* array, int count);

// Load a strand file into the graph. Returns new node IDs.
// Caller owns the returned array and must free with ntw_free_string_array().
char** ntw_load_strand(const char* strandId, int* outCount);

// Get all available template strands. Caller owns the returned array.
NTWAvailableStrandInfo* ntw_get_available_template_strands(int* outCount);

// Load the demo strand into the graph. Returns new node IDs.
// Caller owns the returned array and must free with ntw_free_string_array().
char** ntw_load_demo_strand(int* outCount);

// Free a string array returned by ntw_load_strand.
void ntw_free_string_array(char** array, int count);

// Delete a strand by ID (removes files from disk).
void ntw_delete_strand(const char* strandId);

// Rename a strand by ID.
void ntw_rename_strand(const char* strandId, const char* newName);

// Register a callback that fires when strands are updated.
typedef void (*NTWStrandsUpdatedCallback)(void);
void ntw_register_strands_updated_callback(NTWStrandsUpdatedCallback callback);

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
// MARK: - Text Source State
// ---------------------------------------------------------------------------

typedef struct {
    const char* text;    // Owned by caller (strdup'd) - must free
    int fontSize;
    int isTextSource;    // 1 if this connectable is a TextSource
    float xPosition;     // 0.0 - 1.0
    float yPosition;     // 0.0 - 1.0
    int fontIndex;       // Current font selector index
    int fontCount;       // Total available fonts
    const char** fontNames; // Array of font name strings (owned, must free)
} NTWTextSourceState;

NTWTextSourceState ntw_get_text_source_state(const char* sourceId);
void ntw_set_text_source_text(const char* sourceId, const char* text);
void ntw_set_text_source_font_size(const char* sourceId, int fontSize);
void ntw_set_text_source_font_index(const char* sourceId, int fontIndex);
void ntw_set_text_source_position(const char* sourceId, float x, float y);
void ntw_center_text_source(const char* sourceId);

// ---------------------------------------------------------------------------
// MARK: - Icon Source State
// ---------------------------------------------------------------------------

typedef struct {
    int isIconSource;       // 1 if this connectable is an IconSource
    int selectedIndex;      // Currently selected icon index
    int iconCount;          // Total available icons
    const char** iconNames;     // Array of icon name strings (owned, must free)
    const char** iconPaths;     // Array of absolute file paths (owned, must free)
    const char** iconCategories; // Array of category strings (owned, must free)
} NTWIconSourceState;

NTWIconSourceState ntw_get_icon_source_state(const char* sourceId);
void ntw_free_icon_source_state(NTWIconSourceState* state);
void ntw_set_icon_source_icon(const char* sourceId, int iconIndex);

// ---------------------------------------------------------------------------
// MARK: - Available Non-Shader Sources (for sidebar browser)
// ---------------------------------------------------------------------------

typedef struct {
    const char* id;       // Owned by caller - must free
    const char* name;     // Owned by caller - must free
    const char* icon;     // SF Symbol name - owned by caller - must free
    int sourceType;       // VideoSourceType enum value
    int isFavorited;      // 1 if this source type is favorited
} NTWAvailableNonShaderSourceInfo;

NTWAvailableNonShaderSourceInfo* ntw_get_available_non_shader_sources(int* outCount);
void ntw_free_available_non_shader_source_info_array(NTWAvailableNonShaderSourceInfo* array, int count);

// ---------------------------------------------------------------------------
// MARK: - Favorites (Shader Types & Source Types)
// ---------------------------------------------------------------------------

void ntw_toggle_favorite_shader_type(int shaderTypeRaw);
int ntw_is_shader_type_favorited(int shaderTypeRaw);
void ntw_toggle_favorite_source_type(int sourceType);
int ntw_is_source_type_favorited(int sourceType);

// ---------------------------------------------------------------------------
// MARK: - Strand Sharing & Community
// ---------------------------------------------------------------------------

// Callback types for async strand operations.
typedef void (*NTWShareStrandCallback)(bool success, const char* slugOrError);

typedef struct {
    const char* slug;
    const char* title;
    const char* description;
    const char* previewUrl;
    const char* author;
    int upvotes;
    int downvotes;
    int score;
    int views;
    int opens;
    const char* createdAt;
    const char* userVote;  // "up", "down", or "" for none
} NTWSharedStrandInfo;

typedef void (*NTWFeedCallback)(bool success, NTWSharedStrandInfo* strands, int count, int totalPages);
typedef void (*NTWVoteCallback)(bool success, int upvotes, int downvotes, int score, const char* userVote);
typedef void (*NTWFetchStrandCallback)(bool success, const char* jsonOrError);

// Share a strand (async). Serializes strand JSON on engine thread, then uploads via HTTP.
// previewPngData/previewLen: optional PNG screenshot data. Pass NULL/0 to skip.
// Callback receives slug on success, error message on failure.
void ntw_share_strand(const char* nodeId, const char* title, const char* description,
                      const char* author, const void* previewPngData, int previewLen,
                      NTWShareStrandCallback callback);

// Fetch community feed (async). page is 1-based.
// voterId: unique voter ID for tracking user's votes on each strand.
void ntw_fetch_community_feed(int page, int limit, const char* voterId, NTWFeedCallback callback);

// Free an array of NTWSharedStrandInfo returned by feed callback.
void ntw_free_shared_strand_info_array(NTWSharedStrandInfo* array, int count);

// Vote on a strand (async). vote: "up", "down", or "none" to remove vote.
void ntw_vote_strand(const char* slug, const char* voterId, const char* vote, NTWVoteCallback callback);

// Fetch a shared strand's JSON by slug (async).
void ntw_fetch_shared_strand(const char* slug, NTWFetchStrandCallback callback);

// Load a strand from a JSON string into the graph.
// Returns new node IDs. Caller owns the returned array and must free with ntw_free_string_array().
char** ntw_load_strand_from_json(const char* jsonStr, int* outCount);

// Get serialized strand JSON for a node. Caller must free with ntw_free_string().
char* ntw_get_strand_json_for_node(const char* nodeId);

// ---------------------------------------------------------------------------
// MARK: - MIDI Control
// ---------------------------------------------------------------------------

// Begin MIDI learning for a parameter (next MIDI input will bind to it).
void ntw_begin_midi_learning(const char* paramId);

// Stop MIDI learning without binding.
void ntw_stop_midi_learning(void);

// Returns 1 if currently in MIDI learning mode.
int ntw_is_midi_learning(void);

// Returns the param ID currently being learned, or NULL if not learning.
// Caller must free.
char* ntw_get_midi_learning_param_id(void);

// Remove the MIDI binding from a parameter.
void ntw_remove_midi_binding(const char* paramId);

// Callback fired when a MIDI binding is learned.
typedef void (*NTWMidiLearnedCallback)(const char* paramId, const char* descriptor);
void ntw_register_midi_learned_callback(NTWMidiLearnedCallback callback);

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
