#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QVector>
#include <atomic>

class FrameExchange;

// One entry of the engine's catalog. Shader sources and effects carry their
// shader type; inputs (text, icon, image, webcam) carry a VideoSourceType.
struct CatalogItem {
    QString name;
    QString category;
    int     typeRaw  = 0;
    bool    isSource = false;
    bool    isInput  = false;
    bool    favorite = false;

    // Drag payload, also used as a stable key: "shader:N", "source:N", "input:N".
    QString payload() const;
};

namespace VideoSourceKind { enum { Webcam = 0, File = 1, Image = 2, Icon = 3, Text = 5 }; }

struct GraphNode {
    QString id;
    QString name;
    bool    isShader   = false;
    int     typeRaw    = 0;
    int     inputCount = 0;
    QPointF position;
    bool    active = true;
    bool    hasAuxOutput = false;
    bool    audioReactive = false;
};

// Shaders with an auxiliary output expose it on this output slot.
constexpr int kAuxOutputSlot = 10;

struct GraphEdge {
    QString id;
    QString startId;
    QString endId;
    int     connectionType = 0;  // 0 = shader start, 1 = source start
    int     outputSlot = 0;
    int     inputSlot  = 0;
};

struct StrandItem {
    QString id;
    QString name;
    QString imagePath;
};

struct LibraryItem {
    QString id;
    QString name;
    QString category;
    QString thumbnailPath;  // empty until downloaded
    bool    downloaded  = false;
    bool    downloading = false;
    bool    paused      = false;
    float   progress    = 0;
};

// A strand shared to the nottawa.app community.
struct SharedStrand {
    QString slug;
    QString title;
    QString description;
    QString previewUrl;
    QString author;
    int     upvotes = 0, downvotes = 0, score = 0, views = 0, opens = 0;
    QString createdAt;
    QString userVote;  // "up", "down" or empty
};

struct OscillatorInfo {
    QString paramId;
    QString paramName;
    QString ownerName;
    QString connectableId;
    bool    enabled = false;
    float   frequency = 0, minOutput = 0, maxOutput = 1;
    int     waveShape = 0;
    float   paramMin = 0, paramMax = 1, value = 0;
};

// One poll of the audio analysis (ntw_get_extended_audio_analysis).
struct AudioAnalysis {
    float rms = 0, lows = 0, mids = 0, highs = 0, bpm = 120, beatPulse = 0;
    int   bpmMode = 0;          // 0 Auto, 1 Manual, 2 Link
    bool  bpmLocked = false, bpmEnabled = true;
    float bpmNudge = 0;
    int   smoothingMode = 0;    // 0 Exponential, 1 Moving average, 2 None, 3 Peak hold
    float frequencyRelease = 0, frequencyScale = 1, loudnessRelease = 0, loudnessScale = 1;
    quint32 beatCount = 0;
    bool  audioActive = false;
    int   sourceType = 0;       // 0 Mic, 1 File, 2 System
    QVector<float> melSpectrum;
    QVector<float> waveform;
};

struct FileAudioState {
    float volume = 1, position = 0, duration = 0;
    bool  paused = false, isFileSource = false;
    int   track = -1;
};

struct AudioTrack {
    QString name;
    int     index = 0;
};

// One app preference (ntw_get_settings; docs/SETTINGS.md).
struct SettingInfo {
    enum Type { Bool = 0, Int = 1, Float = 2, Choice = 3, String = 4, Path = 5, Action = 6 };
    QString id, section, label, help, unit;
    QString group, keywords, dependsOn;
    Type    type = Bool;
    double  min = 0, max = 1, step = 0;
    int     decimals = 0;
    double  defaultNumber = 0;
    QString defaultString;
    QStringList choiceIds, choiceLabels;
    int     restart = 0;    // platform bits where a change needs a restart
    int     platforms = 3;  // 1 macOS, 2 Windows, 3 both
};

struct AudioSourceItem {
    QString id;
    QString name;
};

struct ParamInfo {
    enum Type { Standard = 0, Int = 1, Bool = 2, Color = 3, Hidden = 4, Text = 5 };
    QString     id;
    QString     name;
    int         type = Standard;
    float       value = 0, min = 0, max = 1;
    int         intValue = 0;
    bool        boolValue = false;
    QString     stringValue;
    QStringList options;
    bool        favorite = false;
    QColor      color;
    bool        hidden = false;       // hidden at runtime by its shader
    // Paired WaveformOscillator, if any.
    bool        hasOscillator = false;
    bool        oscEnabled = false;
    float       oscFrequency = 0, oscMin = 0, oscMax = 1;
    int         oscShape = 0;
    // Audio driver.
    bool        hasDriver = false;
    QString     driverName;
    float       driverShift = 0, driverScale = 1;
    // MIDI binding.
    bool        hasMidi = false;
    QString     midiDescriptor;

    bool driven() const { return oscEnabled || hasDriver; }
};

struct TextSourceState {
    bool        valid = false;  // the node is a text source
    QString     text;
    int         fontSize = 0;
    float       x = 0.5f, y = 0.5f;  // 0..1, y down
    int         fontIndex = 0;
    QStringList fonts;
};

struct ShaderCompileResult {
    bool    ok = false;
    QString log;
    int     errorLine = 0;  // first source line the log refers to, or 0
};

struct FileSourceState {
    bool  valid = false;  // the node plays a video file (file or library source)
    float speed = 1, position = 0, duration = 0;
    bool  playing = false;
    float volume = 0.5f;
    bool  muted = true;
    // Frame cache: complete (in use when enabled), frames cached so far.
    bool  cacheAvailable = false;
    bool  cacheEnabled = true;
    int   cachedFrames = 0;
};

struct IconEntry {
    QString name;
    QString path;
    QString category;
};

struct IconSourceState {
    bool               valid = false;  // the node is an icon source
    int                selected = -1;
    QVector<IconEntry> icons;
};

struct OptionalEffect {
    QString name;
    bool    enabled = false;
    int     index = 0;
};

// Qt wrapper over src/Bridge/nottawa_bridge.h, the same C API the macOS
// SwiftUI app drives the engine with. Call from the GUI thread.
class EngineBridge : public QObject {
    Q_OBJECT
public:
    static EngineBridge* instance();

    // shareContext: the UI's native GL context (HGLRC), current on the
    // calling thread. Blocks while services load.
    bool start(void* shareContext, const QString& dataPath);
    void stop();
    bool isRunning() const { return running_; }
    float fps() const;
    QSize resolution() const;
    // Output resolution preset: 0 240p, 1 360p, 2 480p, 3 720p, 4 1080p,
    // 5 1440p, 6 4K.
    int   resolutionIndex() const;
    void  setResolutionIndex(int index);

    QVector<CatalogItem> effects() const;
    QVector<CatalogItem> sources() const;
    QVector<CatalogItem> inputs() const;
    void toggleFavorite(const CatalogItem& item);
    QVector<GraphNode>   nodes() const;
    QVector<GraphEdge>   edges() const;
    // Deepest node with no outgoing connection, preferring shaders — the same
    // rule as NodeEditorViewModel's default stage output.
    QString outputNodeId() const;

    QString addEffect(int typeRaw, const QString& connectFromId);
    QString addSource(int typeRaw);
    // Adds a text, icon, image or webcam source; images need `path`.
    QString addInput(int videoSourceType, const QString& path = {});
    QString connectNodes(const QString& startId, const QString& endId, int outputSlot, int inputSlot);
    void    removeConnection(const QString& connectionId);
    void    removeNode(const QString& connectableId);
    void    setNodePosition(const QString& connectableId, QPointF pos);
    void    selectNode(const QString& connectableId);
    void    setNodeActive(const QString& connectableId, bool active);
    QString blendBetween(const QString& id1, const QString& id2);
    // Swaps a node for another catalog type, keeping its connections.
    // Returns the replacement's id.
    QString replaceNode(const QString& connectableId, int typeRaw, bool isSource);
    void    copyNodes(const QStringList& ids);
    QStringList pasteNodes();
    void    undo();
    void    redo();
    bool    canUndo() const;
    bool    canRedo() const;
    // Whether disconnecting an input clears the node's frame (macOS's
    // "Clear on Disconnect").
    void    setClearOnDisconnect(bool on);
    // Bypassed shaders pass their input through unprocessed.
    void    setShaderBypassed(const QString& shaderId, bool bypassed);
    bool    isShaderBypassed(const QString& shaderId) const;
    float   currentBpm() const;
    void    clearGraph();
    bool    loadWorkspace(const QString& path);
    void    saveWorkspace(const QString& path, const QString& name);

    QVector<ParamInfo> parameters(const QString& connectableId) const;
    void setParameter(const QString& paramId, float value);
    void setParameter(const QString& paramId, int value);
    void setParameter(const QString& paramId, bool value);
    void setParameter(const QString& paramId, const QString& value);
    void setParameterColor(const QString& paramId, const QColor& color);
    void resetParameter(const QString& paramId);
    void resetAllParameters(const QString& connectableId);
    void toggleParameterFavorite(const QString& paramId);

    // Audio drivers: an audio feature (by name) driving a parameter.
    QStringList audioDriverNames() const;
    void setParameterDriver(const QString& paramId, const QString& audioFeature);
    void removeParameterDriver(const QString& paramId);
    void setParameterDriverShift(const QString& paramId, float shift);
    void setParameterDriverScale(const QString& paramId, float scale);

    // MIDI learn: the next control moved binds to the parameter
    // (midiLearned fires).
    void beginMidiLearning(const QString& paramId);
    void stopMidiLearning();
    void removeMidiBinding(const QString& paramId);
    struct MidiInput { QString name; int port = 0; bool open = false; };
    struct MidiAssignment { QString paramId, paramName, ownerName, descriptor; };
    QVector<MidiInput>      midiInputs() const;
    QVector<MidiAssignment> midiAssignments() const;
    // Eases MIDI control changes over a duration (toggles aren't eased).
    bool  midiSmoothingEnabled() const;
    void  setMidiSmoothingEnabled(bool on);
    float midiSmoothingMs() const;
    void  setMidiSmoothingMs(float ms);

    // Optional effects built into every node (HSB, Blur, Mirror...).
    QVector<OptionalEffect> optionalEffects(const QString& connectableId) const;
    void toggleOptionalEffect(const QString& connectableId, int index);
    QVector<ParamInfo> optionalEffectParameters(const QString& connectableId, int index) const;

    TextSourceState textSourceState(const QString& sourceId) const;
    void setTextSourceText(const QString& sourceId, const QString& text);
    void setTextSourceFontSize(const QString& sourceId, int size);
    void setTextSourceFont(const QString& sourceId, int fontIndex);
    void setTextSourcePosition(const QString& sourceId, float x, float y);
    void centerTextSource(const QString& sourceId);

    // The shader's .frag file on disk (empty for non-shaders).
    QString shaderFilePath(const QString& shaderId) const;
    // Live GLSL editing: session-only fragment overrides.
    QString shaderSource(const QString& shaderId) const;
    ShaderCompileResult setShaderSource(const QString& shaderId, const QString& source);
    void    revertShaderSource(const QString& shaderId);
    bool    shaderHasOverride(const QString& shaderId) const;

    FileSourceState fileSourceState(const QString& sourceId) const;
    void setFileSourcePlaying(const QString& sourceId, bool playing);
    void setFileSourcePosition(const QString& sourceId, float position);
    void setFileSourceSpeed(const QString& sourceId, float speed);
    void setFileSourceVolume(const QString& sourceId, float volume);
    void setFileSourceMuted(const QString& sourceId, bool muted);
    void setFileSourceUseCache(const QString& sourceId, bool on);

    IconSourceState iconSourceState(const QString& sourceId) const;
    void setIconSourceIcon(const QString& sourceId, int index);

    void clearFeedbackBuffer(const QString& shaderId);
    void resetBouncePosition(const QString& shaderId);

    // Download state of a library video source node (waiting or
    // downloading: `downloading`).
    struct LibraryDownload { bool downloading = false; bool paused = false; float progress = 0; };
    LibraryDownload librarySourceState(const QString& sourceId) const;
    QVector<StrandItem> strands() const;
    // Strands bundled with the app (loadStrand takes their ids too).
    QVector<StrandItem> templateStrands() const;
    QString     strandsFolder() const;
    // Saves the chain ending at `nodeId` as a strand file; `previewPath` is
    // an optional thumbnail already written to disk.
    bool        saveStrand(const QString& nodeId, const QString& path, const QString& name,
                           const QString& previewPath = {});
    // Adds a saved strand's nodes to the graph; returns their ids.
    QStringList loadStrand(const QString& strandId);
    void        deleteStrand(const QString& strandId);
    void        renameStrand(const QString& strandId, const QString& name);

    // The nottawa.app video library. fetchLibrary() runs in the background;
    // libraryChanged fires when it completes.
    void fetchLibrary();
    QVector<LibraryItem> libraryItems() const;
    // Adds a source for a library video (downloading it if needed).
    QString addLibrarySource(const QString& libraryFileId);
    void    pauseLibraryDownload(const QString& libraryFileId);
    void    resumeLibraryDownload(const QString& libraryFileId);

    // Community strands (nottawa.app). Each request answers with the
    // matching signal below, on the GUI thread. voterId identifies this
    // install's votes.
    void fetchCommunityFeed(int page, int limit, const QString& voterId);
    void voteOnStrand(const QString& slug, const QString& voterId, const QString& vote);  // "up"/"down"/"none"
    void fetchSharedStrand(const QString& slug);
    // Publishes the chain ending at `nodeId` to the community.
    void shareStrand(const QString& nodeId, const QString& title, const QString& description,
                     const QString& author, const QByteArray& previewPng);
    QStringList loadStrandJson(const QString& json);

    // Enabled waveform oscillators across all nodes.
    QVector<OscillatorInfo> activeOscillators() const;
    void setOscillatorEnabled(const QString& paramId, bool on);
    void setOscillatorFrequency(const QString& paramId, float frequency);
    void setOscillatorRange(const QString& paramId, float minOutput, float maxOutput);
    void setOscillatorWaveShape(const QString& paramId, int shape);

    AudioAnalysis audioAnalysis() const;
    FileAudioState fileAudioState() const;
    QVector<AudioTrack> sampleTracks() const;
    void setBpmMode(int mode);
    void setBpmLocked(bool on);
    void setBpmEnabled(bool on);
    void setBpm(float bpm);
    void nudgeBpm(float delta);
    // A tap now (call it at the key or button press), or at `seconds` on the
    // steady clock (tests). Emits tapped().
    void tapBpm(double seconds = -1);

    // App preferences, shared with the macOS app (docs/SETTINGS.md).
    QVector<SettingInfo> settings() const;
    double  settingNumber(const QString& id) const;
    QString settingString(const QString& id) const;
    void    setSetting(const QString& id, double value);
    void    setSetting(const QString& id, const QString& value);
    void    resetSettings(const QString& section = QString());

    // Testing: ntw_get_beat_probe.
    struct BeatProbe { double phase = 0, bpm = 0, fileAudioSeconds = -1, videoSeconds = -1; };
    BeatProbe beatProbe(const QString& videoSourceId = QString()) const;
    void setSmoothingMode(int mode);
    void setFrequencyRelease(float v);
    void setFrequencyScale(float v);
    void setLoudnessRelease(float v);
    void setLoudnessScale(float v);
    void selectSampleTrack(int index);
    void setFileAudioVolume(float v);
    void toggleFileAudioPause();
    void setFileAudioPosition(float p);

    QVector<AudioSourceItem> audioSources() const;
    QString selectedAudioSource() const;
    void    selectAudioSource(const QString& id);
    // Starts or stops capture on the selected audio source.
    void    toggleAudio();
    bool    isAudioActive() const;

    // Browser previews: small standalone instances of a catalog type,
    // shared like nodes. These block until the engine thread runs them
    // (create compiles a shader), so they're safe and meant to be called
    // from a worker thread.
    static QString createPreview(int typeRaw, bool isSource);
    static void    destroyPreview(const QString& previewId);
    // Destroys every preview, and the shared source effect previews use.
    static void    destroyAllPreviews();
    // Effect previews process this source's output (or any active one).
    void setPreferredPreviewSource(const QString& sourceId);

    // Texture sharing is reference counted per node: several views (stage,
    // node previews) may show the same node.
    void retainFrames(const QString& connectableId);
    void releaseFrames(const QString& connectableId);
    // Null until the engine has produced the first shared frame.
    FrameExchange* frames(const QString& connectableId) const;

signals:
    void graphChanged();
    void strandsChanged();
    void libraryChanged();
    // The output resolution changed (from any thread; delivered queued).
    void resolutionChanged();
    void midiLearned(const QString& paramId, const QString& descriptor);
    void communityFeedFetched(bool ok, const QVector<SharedStrand>& strands, int page, int totalPages);
    void strandVoted(const QString& slug, bool ok, int upvotes, int downvotes, int score, const QString& userVote);
    void sharedStrandFetched(const QString& slug, bool ok, const QString& jsonOrError);
    void strandShared(bool ok, const QString& slugOrError);
    void tapped(float bpm);  // the tapped tempo so far (0 until two taps)
    void settingChanged(const QString& id);  // an app preference changed (from anywhere)

private:
    EngineBridge() = default;
    std::atomic<bool> running_{false};  // read by preview worker threads
    QHash<QString, int> shareCounts_;
};
