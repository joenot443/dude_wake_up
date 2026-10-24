#include "EngineBridge.h"

#include "nottawa_bridge.h"

#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <cstdlib>
#include <deque>
#include <mutex>

namespace {

QString takeString(char* s) {
    QString out = s ? QString::fromUtf8(s) : QString();
    if (s) ntw_free_string(s);
    return out;
}

QString str(const char* s) { return s ? QString::fromUtf8(s) : QString(); }

void onGraphChanged() {
    QMetaObject::invokeMethod(EngineBridge::instance(), &EngineBridge::graphChanged, Qt::QueuedConnection);
}

void onResolutionChanged() {
    QMetaObject::invokeMethod(EngineBridge::instance(), &EngineBridge::resolutionChanged, Qt::QueuedConnection);
}

void onLibraryChanged() {
    QMetaObject::invokeMethod(EngineBridge::instance(), &EngineBridge::libraryChanged, Qt::QueuedConnection);
}

// The community callbacks carry no context and arrive on worker threads;
// requests of each kind are answered in order, so queue what each needs.
std::mutex g_communityMutex;
std::deque<int>     g_feedPages;
std::deque<QString> g_voteSlugs;
std::deque<QString> g_fetchSlugs;

template <typename T>
T popFront(std::deque<T>& queue) {
    std::lock_guard<std::mutex> lk(g_communityMutex);
    if (queue.empty()) return T();
    T value = queue.front();
    queue.pop_front();
    return value;
}

void onFeed(bool ok, NTWSharedStrandInfo* arr, int count, int totalPages) {
    QVector<SharedStrand> strands;
    for (int i = 0; i < count; ++i) {
        const auto& s = arr[i];
        strands.push_back({str(s.slug), str(s.title), str(s.description), str(s.previewUrl), str(s.author),
                           s.upvotes, s.downvotes, s.score, s.views, s.opens, str(s.createdAt), str(s.userVote)});
    }
    ntw_free_shared_strand_info_array(arr, count);
    const int page = popFront(g_feedPages);
    QMetaObject::invokeMethod(EngineBridge::instance(), [=] {
        emit EngineBridge::instance()->communityFeedFetched(ok, strands, page, totalPages);
    }, Qt::QueuedConnection);
}

void onVote(bool ok, int up, int down, int score, const char* userVote) {
    const QString vote = str(userVote);
    if (userVote) ntw_free_string(const_cast<char*>(userVote));
    const QString slug = popFront(g_voteSlugs);
    QMetaObject::invokeMethod(EngineBridge::instance(), [=] {
        emit EngineBridge::instance()->strandVoted(slug, ok, up, down, score, vote);
    }, Qt::QueuedConnection);
}

void onFetchStrand(bool ok, const char* jsonOrError) {
    const QString text = str(jsonOrError);
    if (jsonOrError) ntw_free_string(const_cast<char*>(jsonOrError));
    const QString slug = popFront(g_fetchSlugs);
    QMetaObject::invokeMethod(EngineBridge::instance(), [=] {
        emit EngineBridge::instance()->sharedStrandFetched(slug, ok, text);
    }, Qt::QueuedConnection);
}

void onShare(bool ok, const char* slugOrError) {
    const QString text = str(slugOrError);
    if (slugOrError) ntw_free_string(const_cast<char*>(slugOrError));
    QMetaObject::invokeMethod(EngineBridge::instance(), [=] {
        emit EngineBridge::instance()->strandShared(ok, text);
    }, Qt::QueuedConnection);
}

void onMidiLearned(const char* paramId, const char* descriptor) {
    const QString id = str(paramId), desc = str(descriptor);
    QMetaObject::invokeMethod(EngineBridge::instance(), [=] {
        emit EngineBridge::instance()->midiLearned(id, desc);
    }, Qt::QueuedConnection);
}

void onStrandsChanged() {
    QMetaObject::invokeMethod(EngineBridge::instance(), &EngineBridge::strandsChanged, Qt::QueuedConnection);
}

const char* const kEffectCategories[] = {"Basic", "Mix", "Transform", "Filter", "Mask", "Glitch"};

// Shaders whose aux output pin always shows, as macOS lists them (the
// engine only reports one once "Allow Auxillary" is on; connecting to the
// pin turns that on): Feedback, Luma Mask Maker, Color Key Mask Maker.
bool shaderTypeHasAuxOutput(int type) { return type == 7 || type == 41 || type == 45; }

} // anon

QString CatalogItem::payload() const {
    return QString("%1:%2").arg(isInput ? "input" : isSource ? "source" : "shader").arg(typeRaw);
}

EngineBridge* EngineBridge::instance() {
    static EngineBridge bridge;
    return &bridge;
}

bool EngineBridge::start(void* shareContext, const QString& dataPath) {
    if (running_) return true;
    ntw_set_data_path(dataPath.toUtf8().constData());
    ntw_set_share_context(shareContext);
    if (!ntw_engine_init()) return false;
    ntw_register_graph_changed_callback(&onGraphChanged);
    ntw_register_strands_updated_callback(&onStrandsChanged);
    ntw_register_library_update_callback(&onLibraryChanged);
    ntw_register_resolution_changed_callback(&onResolutionChanged);
    ntw_register_midi_learned_callback(&onMidiLearned);
    ntw_engine_start();
    running_ = true;
    return true;
}

void EngineBridge::stop() {
    if (!running_) return;
    // The session: changes since the last (debounced or periodic) save.
    ntw_save_default_config();
    ntw_engine_stop();
    running_ = false;
}

float EngineBridge::fps() const { return running_ ? ntw_engine_fps() : 0.f; }

int  EngineBridge::resolutionIndex() const { return running_ ? ntw_get_resolution_setting_index() : 3; }
void EngineBridge::setResolutionIndex(int index) { if (running_) ntw_set_resolution(index); }

QSize EngineBridge::resolution() const {
    return QSize(int(ntw_get_resolution_width()), int(ntw_get_resolution_height()));
}

QVector<CatalogItem> EngineBridge::effects() const {
    QVector<CatalogItem> out;
    for (int c = 1; c <= 6; ++c) {
        int n = 0;
        NTWAvailableShaderInfo* arr = ntw_get_available_shaders(c, &n);
        for (int i = 0; i < n; ++i)
            out.push_back({str(arr[i].name), kEffectCategories[c - 1], arr[i].shaderTypeRaw, false, false,
                           arr[i].isFavorited != 0});
        ntw_free_available_shader_info_array(arr, n);
    }
    return out;
}

QVector<CatalogItem> EngineBridge::sources() const {
    QVector<CatalogItem> out;
    int n = 0;
    NTWAvailableShaderInfo* arr = ntw_get_available_shader_sources(&n);
    for (int i = 0; i < n; ++i)
        out.push_back({str(arr[i].name), str(arr[i].category), arr[i].shaderTypeRaw, true, false,
                       arr[i].isFavorited != 0});
    ntw_free_available_shader_info_array(arr, n);
    return out;
}

QVector<CatalogItem> EngineBridge::inputs() const {
    QVector<CatalogItem> out;
    int n = 0;
    NTWAvailableNonShaderSourceInfo* arr = ntw_get_available_non_shader_sources(&n);
    for (int i = 0; i < n; ++i)
        out.push_back({str(arr[i].name), "Input", arr[i].sourceType, true, true, arr[i].isFavorited != 0});
    ntw_free_available_non_shader_source_info_array(arr, n);
    return out;
}

void EngineBridge::toggleFavorite(const CatalogItem& item) {
    // Inputs share the source favourites, offset by 1000 (as on macOS).
    if (item.isInput) ntw_toggle_favorite_source_type(1000 + item.typeRaw);
    else if (item.isSource) ntw_toggle_favorite_source_type(item.typeRaw);
    else ntw_toggle_favorite_shader_type(item.typeRaw);
}

QString EngineBridge::addInput(int type, const QString& path) {
    switch (type) {
    case VideoSourceKind::Text:   return takeString(ntw_add_text_video_source("Text"));
    case VideoSourceKind::Icon:   return takeString(ntw_add_icon_video_source("Icon"));
    case VideoSourceKind::Webcam: return takeString(ntw_add_webcam_video_source("Webcam", 0));
    case VideoSourceKind::Image:
    case VideoSourceKind::File: {
        if (path.isEmpty()) return {};
        const QByteArray name = QFileInfo(path).fileName().toUtf8();
        const QByteArray file = path.toUtf8();
        return takeString(type == VideoSourceKind::File ? ntw_add_file_video_source(name.constData(), file.constData())
                                                        : ntw_add_image_video_source(name.constData(), file.constData()));
    }
    }
    return {};
}

// Bridge calls made while the engine is stopped run GL code on the calling
// thread, which has no context — so these do nothing then.
QString EngineBridge::createPreview(int typeRaw, bool isSource) {
    if (!instance()->isRunning()) return {};
    return takeString(ntw_create_preview(typeRaw, isSource));
}

void EngineBridge::destroyPreview(const QString& id) {
    if (instance()->isRunning()) ntw_destroy_preview(id.toUtf8().constData());
}

void EngineBridge::destroyAllPreviews() {
    if (instance()->isRunning()) ntw_destroy_all_previews();
}

void EngineBridge::setPreferredPreviewSource(const QString& id) {
    ntw_set_preferred_preview_source(id.isEmpty() ? nullptr : id.toUtf8().constData());
}

QVector<GraphNode> EngineBridge::nodes() const {
    QVector<GraphNode> out;
    int n = 0;
    NTWConnectableInfo* shaders = ntw_get_all_shaders(&n);
    for (int i = 0; i < n; ++i) {
        const auto& s = shaders[i];
        const QByteArray id(s.id);
        out.push_back({str(s.id), str(s.name), true, s.shaderTypeRaw, s.inputCount, QPointF(s.posX, s.posY),
                       ntw_get_connectable_active(id.constData()), shaderTypeHasAuxOutput(s.shaderTypeRaw),
                       ntw_is_audio_reactive(id.constData())});
    }
    ntw_free_connectable_info_array(shaders, n);
    NTWVideoSourceInfo* srcs = ntw_get_all_video_sources(&n);
    for (int i = 0; i < n; ++i) {
        const auto& s = srcs[i];
        out.push_back({str(s.id), str(s.name), false, s.sourceType, s.inputCount, QPointF(s.posX, s.posY),
                       ntw_get_connectable_active(s.id), false, ntw_is_audio_reactive(s.id)});
    }
    ntw_free_video_source_info_array(srcs, n);
    return out;
}

EngineBridge::LibraryDownload EngineBridge::librarySourceState(const QString& sourceId) const {
    LibraryDownload out;
    if (!running_) return out;
    NTWLibrarySourceState s = ntw_get_library_source_state(sourceId.toUtf8().constData());
    out.downloading = s.state == 0 || s.state == 1;
    out.paused = s.isPaused != 0;
    out.progress = s.progress;
    std::free(const_cast<char*>(s.libraryFileId));
    return out;
}

QVector<StrandItem> EngineBridge::strands() const {
    QVector<StrandItem> out;
    int n = 0;
    NTWAvailableStrandInfo* arr = ntw_get_available_strands(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].id), str(arr[i].name), str(arr[i].imagePath)});
    ntw_free_available_strand_info_array(arr, n);
    return out;
}

QVector<StrandItem> EngineBridge::templateStrands() const {
    QVector<StrandItem> out;
    int n = 0;
    NTWAvailableStrandInfo* arr = ntw_get_available_template_strands(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].id), str(arr[i].name), str(arr[i].imagePath)});
    ntw_free_available_strand_info_array(arr, n);
    std::sort(out.begin(), out.end(), [](const StrandItem& a, const StrandItem& b) {
        return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
    });
    return out;
}

QString EngineBridge::strandsFolder() const { return takeString(ntw_get_strands_folder_path()); }

bool EngineBridge::saveStrand(const QString& nodeId, const QString& path, const QString& name,
                              const QString& previewPath) {
    const QByteArray preview = previewPath.toUtf8();
    return ntw_save_strand_from_node_with_preview(nodeId.toUtf8().constData(), path.toUtf8().constData(),
                                                  name.toUtf8().constData(),
                                                  previewPath.isEmpty() ? nullptr : preview.constData());
}

QStringList EngineBridge::loadStrand(const QString& strandId) {
    int n = 0;
    char** ids = ntw_load_strand(strandId.toUtf8().constData(), &n);
    QStringList out;
    for (int i = 0; i < n; ++i) out << str(ids[i]);
    ntw_free_string_array(ids, n);
    return out;
}

void EngineBridge::deleteStrand(const QString& id) { ntw_delete_strand(id.toUtf8().constData()); }

void EngineBridge::renameStrand(const QString& id, const QString& name) {
    ntw_rename_strand(id.toUtf8().constData(), name.toUtf8().constData());
}

void EngineBridge::fetchLibrary() { if (running_) ntw_fetch_library_files(); }

QVector<LibraryItem> EngineBridge::libraryItems() const {
    QVector<LibraryItem> out;
    if (!running_) return out;
    int n = 0;
    NTWLibraryFileInfo* arr = ntw_get_library_files(&n);
    for (int i = 0; i < n; ++i) {
        const auto& f = arr[i];
        out.push_back({str(f.id), str(f.name), str(f.category), str(f.thumbnailPath), f.isDownloaded != 0,
                       f.isDownloading != 0, f.isPaused != 0, f.downloadProgress});
    }
    ntw_free_library_file_info_array(arr, n);
    return out;
}

QString EngineBridge::addLibrarySource(const QString& id) {
    return takeString(ntw_add_library_video_source(id.toUtf8().constData()));
}

void EngineBridge::pauseLibraryDownload(const QString& id) { ntw_pause_library_download(id.toUtf8().constData()); }
void EngineBridge::resumeLibraryDownload(const QString& id) { ntw_resume_library_download(id.toUtf8().constData()); }

void EngineBridge::fetchCommunityFeed(int page, int limit, const QString& voterId) {
    { std::lock_guard<std::mutex> lk(g_communityMutex); g_feedPages.push_back(page); }
    ntw_fetch_community_feed(page, limit, voterId.toUtf8().constData(), &onFeed);
}

// Test runs (NOTTAWA_NO_PUBLISH) never reach the public community.
static bool publishingDisabled() { return !qEnvironmentVariableIsEmpty("NOTTAWA_NO_PUBLISH"); }

void EngineBridge::voteOnStrand(const QString& slug, const QString& voterId, const QString& vote) {
    if (publishingDisabled()) return;
    { std::lock_guard<std::mutex> lk(g_communityMutex); g_voteSlugs.push_back(slug); }
    ntw_vote_strand(slug.toUtf8().constData(), voterId.toUtf8().constData(), vote.toUtf8().constData(), &onVote);
}

void EngineBridge::fetchSharedStrand(const QString& slug) {
    { std::lock_guard<std::mutex> lk(g_communityMutex); g_fetchSlugs.push_back(slug); }
    ntw_fetch_shared_strand(slug.toUtf8().constData(), &onFetchStrand);
}

void EngineBridge::shareStrand(const QString& nodeId, const QString& title, const QString& description,
                               const QString& author, const QByteArray& previewPng) {
    if (publishingDisabled()) {
        QMetaObject::invokeMethod(this, [this] { emit strandShared(false, "Sharing is disabled in test runs"); },
                                  Qt::QueuedConnection);
        return;
    }
    ntw_share_strand(nodeId.toUtf8().constData(), title.toUtf8().constData(), description.toUtf8().constData(),
                     author.toUtf8().constData(), previewPng.isEmpty() ? nullptr : previewPng.constData(),
                     int(previewPng.size()), &onShare);
}

QStringList EngineBridge::loadStrandJson(const QString& json) {
    int n = 0;
    char** ids = ntw_load_strand_from_json(json.toUtf8().constData(), &n);
    QStringList out;
    for (int i = 0; i < n; ++i) out << str(ids[i]);
    ntw_free_string_array(ids, n);
    return out;
}

QVector<OscillatorInfo> EngineBridge::activeOscillators() const {
    QVector<OscillatorInfo> out;
    if (!running_) return out;
    int n = 0;
    NTWActiveOscillatorInfo* arr = ntw_get_active_oscillators(&n);
    for (int i = 0; i < n; ++i) {
        const auto& o = arr[i];
        out.push_back({str(o.paramId), str(o.paramName), str(o.ownerName), str(o.connectableId),
                       o.oscillatorEnabled != 0, o.frequency, o.minOutput, o.maxOutput, o.waveShape,
                       o.paramMin, o.paramMax, o.currentValue});
    }
    ntw_free_active_oscillator_array(arr, n);
    return out;
}

void EngineBridge::setOscillatorEnabled(const QString& id, bool on) { ntw_set_oscillator_enabled(id.toUtf8().constData(), on); }
void EngineBridge::setOscillatorFrequency(const QString& id, float f) { ntw_set_oscillator_frequency(id.toUtf8().constData(), f); }
void EngineBridge::setOscillatorRange(const QString& id, float minOutput, float maxOutput) {
    const QByteArray utf8 = id.toUtf8();
    ntw_set_oscillator_min_output(utf8.constData(), minOutput);
    ntw_set_oscillator_max_output(utf8.constData(), maxOutput);
}
void EngineBridge::setOscillatorWaveShape(const QString& id, int shape) { ntw_set_oscillator_wave_shape(id.toUtf8().constData(), shape); }

AudioAnalysis EngineBridge::audioAnalysis() const {
    AudioAnalysis a;
    if (!running_) return a;
    const NTWExtendedAudioAnalysisInfo s = ntw_get_extended_audio_analysis();
    a.rms = s.rms; a.lows = s.lows; a.mids = s.mids; a.highs = s.highs;
    a.bpm = s.bpm; a.beatPulse = s.beatPulse;
    a.bpmMode = s.bpmMode; a.bpmLocked = s.bpmLocked; a.bpmNudge = s.bpmNudge; a.bpmEnabled = s.bpmEnabled;
    a.smoothingMode = s.smoothingMode;
    a.frequencyRelease = s.frequencyRelease; a.frequencyScale = s.frequencyScale;
    a.loudnessRelease = s.loudnessRelease; a.loudnessScale = s.loudnessScale;
    a.beatCount = s.beatCount; a.audioActive = s.audioActive; a.sourceType = s.audioSourceType;
    a.melSpectrum = QVector<float>(s.melSpectrum, s.melSpectrum + qBound(0, s.melSpectrumCount, 13));
    a.waveform = QVector<float>(s.waveform, s.waveform + qBound(0, s.waveformCount, 256));
    return a;
}

FileAudioState EngineBridge::fileAudioState() const {
    FileAudioState f;
    if (!running_) return f;
    const NTWFileAudioState s = ntw_get_file_audio_state();
    f.volume = s.volume; f.paused = s.isPaused; f.position = s.playbackPosition;
    f.duration = s.totalDuration; f.isFileSource = s.isFileSource; f.track = s.selectedTrackIndex;
    return f;
}

QVector<AudioTrack> EngineBridge::sampleTracks() const {
    QVector<AudioTrack> out;
    if (!running_) return out;
    int n = 0;
    NTWAudioTrackInfo* arr = ntw_get_sample_tracks(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].name), arr[i].index});
    ntw_free_audio_track_info_array(arr, n);
    return out;
}

void EngineBridge::setBpmMode(int m) { ntw_set_bpm_mode(m); }
void EngineBridge::setBpmLocked(bool on) { ntw_set_bpm_locked(on); }
void EngineBridge::setBpmEnabled(bool on) { ntw_set_bpm_enabled(on); }
void EngineBridge::setBpm(float bpm) { ntw_set_bpm_value(bpm); }
void EngineBridge::nudgeBpm(float d) { ntw_nudge_bpm(d); }
void EngineBridge::tapBpm() { ntw_tap_bpm(); }
void EngineBridge::setSmoothingMode(int m) { ntw_set_smoothing_mode(m); }
void EngineBridge::setFrequencyRelease(float v) { ntw_set_frequency_release(v); }
void EngineBridge::setFrequencyScale(float v) { ntw_set_frequency_scale(v); }
void EngineBridge::setLoudnessRelease(float v) { ntw_set_loudness_release(v); }
void EngineBridge::setLoudnessScale(float v) { ntw_set_loudness_scale(v); }
void EngineBridge::selectSampleTrack(int i) { ntw_select_sample_track(i); }
void EngineBridge::setFileAudioVolume(float v) { ntw_set_file_audio_volume(v); }
void EngineBridge::toggleFileAudioPause() { ntw_toggle_file_audio_pause(); }
void EngineBridge::setFileAudioPosition(float p) { ntw_set_file_audio_position(p); }

QVector<AudioSourceItem> EngineBridge::audioSources() const {
    QVector<AudioSourceItem> out;
    int n = 0;
    NTWAudioSourceInfo* arr = ntw_get_all_audio_sources(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].id), str(arr[i].name)});
    ntw_free_audio_source_info_array(arr, n);
    return out;
}

QString EngineBridge::selectedAudioSource() const {
    return takeString(const_cast<char*>(ntw_get_selected_audio_source_id()));
}

void EngineBridge::selectAudioSource(const QString& id) { ntw_select_audio_source(id.toUtf8().constData()); }
void EngineBridge::toggleAudio() { ntw_toggle_audio_source(); }
bool EngineBridge::isAudioActive() const { return running_ && ntw_is_audio_active(); }

QVector<GraphEdge> EngineBridge::edges() const {
    QVector<GraphEdge> out;
    int n = 0;
    NTWConnectionInfo* arr = ntw_get_all_connections(&n);
    for (int i = 0; i < n; ++i) {
        const auto& c = arr[i];
        out.push_back({str(c.id), str(c.startId), str(c.endId), c.connectionType, c.outputSlot, c.inputSlot});
    }
    ntw_free_connection_info_array(arr, n);
    return out;
}

QString EngineBridge::outputNodeId() const {
    const auto all = nodes();
    QSet<QString> hasOutgoing;
    for (const auto& e : edges()) hasOutgoing.insert(e.startId);

    QString best;
    int bestDepth = -1;
    QString fallbackSource;
    for (const auto& n : all) {
        if (hasOutgoing.contains(n.id)) continue;
        if (n.isShader) {
            const int depth = ntw_get_shader_depth(n.id.toUtf8().constData());
            if (depth > bestDepth) { bestDepth = depth; best = n.id; }
        } else if (fallbackSource.isEmpty()) {
            fallbackSource = n.id;
        }
    }
    return best.isEmpty() ? fallbackSource : best;
}

QString EngineBridge::addEffect(int typeRaw, const QString& connectFromId) {
    const QString id = takeString(ntw_add_shader(typeRaw));
    if (!id.isEmpty() && !connectFromId.isEmpty()) connectNodes(connectFromId, id, 0, 0);
    return id;
}

QString EngineBridge::connectNodes(const QString& startId, const QString& endId, int outputSlot, int inputSlot) {
    bool fromShader = false;
    for (const auto& n : nodes()) if (n.id == startId) { fromShader = n.isShader; break; }
    // Aux output (slot 10) only produces frames once the shader's
    // "Allow Auxillary" parameter is on, as NodeEditorViewModel does.
    if (outputSlot == kAuxOutputSlot) {
        for (const ParamInfo& p : parameters(startId))
            if (p.name == QLatin1String("Allow Auxillary")) setParameter(p.id, true);
    }
    // connectionType follows the start node: 0 = shader, 1 = source.
    return takeString(ntw_make_connection(startId.toUtf8().constData(), endId.toUtf8().constData(),
                                          fromShader ? 0 : 1, outputSlot, inputSlot));
}

void EngineBridge::removeConnection(const QString& id) { ntw_remove_connection(id.toUtf8().constData()); }
void EngineBridge::removeNode(const QString& id)       { ntw_remove_connectable(id.toUtf8().constData()); }
void EngineBridge::selectNode(const QString& id) {
    if (id.isEmpty()) ntw_deselect_connectable(); else ntw_select_connectable(id.toUtf8().constData());
}
void EngineBridge::setNodePosition(const QString& id, QPointF pos) {
    ntw_set_connectable_position(id.toUtf8().constData(), float(pos.x()), float(pos.y()));
}
void EngineBridge::setNodeActive(const QString& id, bool active) {
    ntw_set_connectable_active(id.toUtf8().constData(), active);
}

QString EngineBridge::blendBetween(const QString& id1, const QString& id2) {
    return takeString(ntw_create_blend_between(id1.toUtf8().constData(), id2.toUtf8().constData()));
}

QString EngineBridge::replaceNode(const QString& id, int typeRaw, bool isSource) {
    const QByteArray utf8 = id.toUtf8();
    return takeString(isSource ? ntw_replace_video_source(utf8.constData(), typeRaw)
                               : ntw_replace_shader(utf8.constData(), typeRaw));
}

void EngineBridge::copyNodes(const QStringList& ids) {
    QVector<QByteArray> utf8;
    QVector<const char*> ptrs;
    for (const QString& id : ids) utf8.push_back(id.toUtf8());
    for (const QByteArray& b : utf8) ptrs.push_back(b.constData());
    ntw_copy_connectables(ptrs.data(), int(ptrs.size()));
}

QStringList EngineBridge::pasteNodes() {
    int n = 0;
    char** arr = ntw_paste_connectables(&n);
    QStringList out;
    for (int i = 0; i < n; ++i) out << str(arr[i]);
    if (arr) ntw_free_string_array(arr, n);
    return out;
}

void EngineBridge::undo() { ntw_undo(); }
void EngineBridge::redo() { ntw_redo(); }

QString EngineBridge::addSource(int typeRaw) { return takeString(ntw_add_shader_video_source(typeRaw)); }
void EngineBridge::clearGraph() { ntw_clear_graph(); }
bool EngineBridge::canUndo() const { return running_ && ntw_can_undo(); }
bool EngineBridge::canRedo() const { return running_ && ntw_can_redo(); }
void EngineBridge::setClearOnDisconnect(bool on) { if (running_) ntw_set_clear_on_disconnect(on ? 1 : 0); }
void EngineBridge::setShaderBypassed(const QString& id, bool on) { ntw_set_shader_bypassed(id.toUtf8().constData(), on ? 1 : 0); }
bool EngineBridge::isShaderBypassed(const QString& id) const {
    return running_ && ntw_get_shader_bypassed(id.toUtf8().constData()) != 0;
}
float EngineBridge::currentBpm() const { return running_ ? ntw_get_current_bpm() : 0.0f; }
bool EngineBridge::loadWorkspace(const QString& path) { return ntw_load_workspace(path.toUtf8().constData()); }

void EngineBridge::saveWorkspace(const QString& path, const QString& name) {
    ntw_save_workspace_to_path(path.toUtf8().constData(), name.toUtf8().constData());
}

namespace {
QVector<ParamInfo> toParams(NTWParameterInfo* arr, int n) {
    QVector<ParamInfo> out;
    for (int i = 0; i < n; ++i) {
        const NTWParameterInfo& p = arr[i];
        ParamInfo info;
        info.id = str(p.id);
        info.name = str(p.name);
        info.type = p.parameterType;
        info.value = p.value;
        info.min = p.minValue;
        info.max = p.maxValue;
        info.intValue = p.intValue;
        info.boolValue = p.boolValue != 0;
        info.stringValue = str(p.stringValue);
        for (int o = 0; o < p.optionCount; ++o) info.options << str(p.options[o]);
        info.favorite = p.isFavorited != 0;
        info.color = QColor::fromRgbF(p.colorR, p.colorG, p.colorB, p.colorA);
        info.hidden = p.hiddenFromNode != 0;
        info.hasOscillator = p.hasOscillator != 0;
        info.oscEnabled = p.oscillatorEnabled != 0;
        info.oscFrequency = p.oscillatorFrequency;
        info.oscMin = p.oscillatorMinOutput;
        info.oscMax = p.oscillatorMaxOutput;
        info.oscShape = p.oscillatorWaveShape;
        info.hasDriver = p.hasDriver != 0;
        info.driverName = str(p.driverName);
        info.driverShift = p.driverShift;
        info.driverScale = p.driverScale;
        info.hasMidi = p.hasMidiBinding != 0;
        info.midiDescriptor = str(p.midiDescriptor);
        out.push_back(info);
    }
    ntw_free_parameter_info_array(arr, n);
    return out;
}
} // anon

QVector<ParamInfo> EngineBridge::parameters(const QString& connectableId) const {
    if (!running_) return {};
    int n = 0;
    NTWParameterInfo* arr = ntw_get_parameters(connectableId.toUtf8().constData(), &n);
    return toParams(arr, n);
}

void EngineBridge::setParameterColor(const QString& id, const QColor& c) {
    ntw_set_parameter_color(id.toUtf8().constData(), c.redF(), c.greenF(), c.blueF(), c.alphaF());
}
void EngineBridge::resetParameter(const QString& id) { ntw_reset_parameter(id.toUtf8().constData()); }
void EngineBridge::resetAllParameters(const QString& id) { ntw_reset_all_parameters(id.toUtf8().constData()); }
void EngineBridge::toggleParameterFavorite(const QString& id) { ntw_toggle_parameter_favorite(id.toUtf8().constData()); }

QStringList EngineBridge::audioDriverNames() const {
    QStringList out;
    int n = 0;
    NTWAudioDriverParam* arr = ntw_get_available_audio_drivers(&n);
    for (int i = 0; i < n; ++i) out << str(arr[i].name);
    ntw_free_audio_driver_param_array(arr, n);
    return out;
}
void EngineBridge::setParameterDriver(const QString& id, const QString& feature) {
    ntw_set_parameter_driver(id.toUtf8().constData(), feature.toUtf8().constData());
}
void EngineBridge::removeParameterDriver(const QString& id) { ntw_remove_parameter_driver(id.toUtf8().constData()); }
void EngineBridge::setParameterDriverShift(const QString& id, float v) { ntw_set_parameter_driver_shift(id.toUtf8().constData(), v); }
void EngineBridge::setParameterDriverScale(const QString& id, float v) { ntw_set_parameter_driver_scale(id.toUtf8().constData(), v); }

void EngineBridge::beginMidiLearning(const QString& id) { ntw_begin_midi_learning(id.toUtf8().constData()); }
void EngineBridge::stopMidiLearning() { ntw_stop_midi_learning(); }
void EngineBridge::removeMidiBinding(const QString& id) { ntw_remove_midi_binding(id.toUtf8().constData()); }

QVector<EngineBridge::MidiInput> EngineBridge::midiInputs() const {
    QVector<MidiInput> out;
    if (!running_) return out;
    int n = 0;
    NTWMidiInputInfo* arr = ntw_get_midi_inputs(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].name), arr[i].port, arr[i].isOpen != 0});
    ntw_free_midi_input_info_array(arr, n);
    return out;
}

QVector<EngineBridge::MidiAssignment> EngineBridge::midiAssignments() const {
    QVector<MidiAssignment> out;
    if (!running_) return out;
    int n = 0;
    NTWMidiAssignmentInfo* arr = ntw_get_midi_assignments(&n);
    for (int i = 0; i < n; ++i)
        out.push_back({str(arr[i].paramId), str(arr[i].paramName), str(arr[i].ownerName), str(arr[i].midiDescriptor)});
    ntw_free_midi_assignment_info_array(arr, n);
    return out;
}

bool  EngineBridge::midiSmoothingEnabled() const { return running_ && ntw_get_midi_smoothing_enabled(); }
void  EngineBridge::setMidiSmoothingEnabled(bool on) { if (running_) ntw_set_midi_smoothing_enabled(on); }
float EngineBridge::midiSmoothingMs() const { return running_ ? ntw_get_midi_smoothing_duration_ms() : 0.0f; }
void  EngineBridge::setMidiSmoothingMs(float ms) { if (running_) ntw_set_midi_smoothing_duration_ms(ms); }

QVector<OptionalEffect> EngineBridge::optionalEffects(const QString& id) const {
    QVector<OptionalEffect> out;
    if (!running_) return out;
    int n = 0;
    NTWOptionalShaderInfo* arr = ntw_get_optional_shaders(id.toUtf8().constData(), &n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].name), arr[i].enabled, arr[i].index});
    ntw_free_optional_shader_info_array(arr, n);
    return out;
}
void EngineBridge::toggleOptionalEffect(const QString& id, int index) {
    ntw_toggle_optional_shader(id.toUtf8().constData(), index);
}
QVector<ParamInfo> EngineBridge::optionalEffectParameters(const QString& id, int index) const {
    if (!running_) return {};
    int n = 0;
    NTWParameterInfo* arr = ntw_get_optional_shader_parameters(id.toUtf8().constData(), index, &n);
    return toParams(arr, n);
}

TextSourceState EngineBridge::textSourceState(const QString& id) const {
    TextSourceState out;
    if (!running_) return out;
    NTWTextSourceState s = ntw_get_text_source_state(id.toUtf8().constData());
    out.valid = s.isTextSource != 0;
    out.text = str(s.text);
    out.fontSize = s.fontSize;
    out.x = s.xPosition;
    out.y = s.yPosition;
    out.fontIndex = s.fontIndex;
    for (int i = 0; i < s.fontCount; ++i) out.fonts << str(s.fontNames[i]);
    // No free function for this struct; its strings are malloc'd.
    std::free(const_cast<char*>(s.text));
    for (int i = 0; i < s.fontCount; ++i) std::free(const_cast<char*>(s.fontNames[i]));
    std::free(const_cast<char**>(s.fontNames));
    return out;
}
void EngineBridge::setTextSourceText(const QString& id, const QString& text) {
    ntw_set_text_source_text(id.toUtf8().constData(), text.toUtf8().constData());
}
void EngineBridge::setTextSourceFontSize(const QString& id, int size) { ntw_set_text_source_font_size(id.toUtf8().constData(), size); }
void EngineBridge::setTextSourceFont(const QString& id, int index) { ntw_set_text_source_font_index(id.toUtf8().constData(), index); }
void EngineBridge::setTextSourcePosition(const QString& id, float x, float y) {
    ntw_set_text_source_position(id.toUtf8().constData(), x, y);
}
void EngineBridge::centerTextSource(const QString& id) { ntw_center_text_source(id.toUtf8().constData()); }

QString EngineBridge::shaderSource(const QString& id) const {
    return takeString(ntw_get_shader_fragment_source(id.toUtf8().constData()));
}
ShaderCompileResult EngineBridge::setShaderSource(const QString& id, const QString& source) {
    NTWShaderCompileResult r = ntw_set_shader_fragment_source(id.toUtf8().constData(), source.toUtf8().constData());
    ShaderCompileResult out{r.ok != 0, str(r.errorLog), r.errorLine};
    ntw_free_shader_compile_result(r);
    return out;
}
void EngineBridge::revertShaderSource(const QString& id) { ntw_revert_shader_source(id.toUtf8().constData()); }
bool EngineBridge::shaderHasOverride(const QString& id) const {
    return running_ && ntw_shader_has_session_override(id.toUtf8().constData());
}

FileSourceState EngineBridge::fileSourceState(const QString& id) const {
    FileSourceState out;
    if (!running_) return out;
    const NTWFileSourceState s = ntw_get_file_source_state(id.toUtf8().constData());
    out.valid = s.isFileSource != 0;
    out.speed = s.speed;
    out.position = s.position;
    out.duration = s.duration;
    out.playing = s.playing != 0;
    out.volume = s.volume;
    out.muted = s.mute != 0;
    out.cacheAvailable = s.cacheAvailable != 0;
    out.cacheEnabled = s.useCachedMode != 0;
    out.cachedFrames = s.cachedFrameCount;
    return out;
}
void EngineBridge::setFileSourcePlaying(const QString& id, bool on) { ntw_set_file_source_playing(id.toUtf8().constData(), on); }
void EngineBridge::setFileSourcePosition(const QString& id, float p) { ntw_set_file_source_position(id.toUtf8().constData(), p); }
void EngineBridge::setFileSourceSpeed(const QString& id, float s) { ntw_set_file_source_speed(id.toUtf8().constData(), s); }
void EngineBridge::setFileSourceVolume(const QString& id, float v) { ntw_set_file_source_volume(id.toUtf8().constData(), v); }
void EngineBridge::setFileSourceUseCache(const QString& id, bool on) {
    ntw_set_file_source_use_cached_mode(id.toUtf8().constData(), on ? 1 : 0);
}
void EngineBridge::setFileSourceMuted(const QString& id, bool on) { ntw_set_file_source_mute(id.toUtf8().constData(), on ? 1 : 0); }

IconSourceState EngineBridge::iconSourceState(const QString& id) const {
    IconSourceState out;
    if (!running_) return out;
    NTWIconSourceState s = ntw_get_icon_source_state(id.toUtf8().constData());
    out.valid = s.isIconSource != 0;
    out.selected = s.selectedIndex;
    for (int i = 0; i < s.iconCount; ++i)
        out.icons.push_back({str(s.iconNames[i]), str(s.iconPaths[i]), str(s.iconCategories[i])});
    ntw_free_icon_source_state(&s);
    return out;
}
void EngineBridge::setIconSourceIcon(const QString& id, int index) { ntw_set_icon_source_icon(id.toUtf8().constData(), index); }

void EngineBridge::clearFeedbackBuffer(const QString& id) { ntw_clear_feedback_buffer(id.toUtf8().constData()); }
void EngineBridge::resetBouncePosition(const QString& id) { ntw_reset_bounce_position(id.toUtf8().constData()); }

void EngineBridge::setParameter(const QString& id, float v)          { ntw_set_parameter_value(id.toUtf8().constData(), v); }
void EngineBridge::setParameter(const QString& id, int v)            { ntw_set_parameter_int(id.toUtf8().constData(), v); }
void EngineBridge::setParameter(const QString& id, bool v)           { ntw_set_parameter_bool(id.toUtf8().constData(), v); }
void EngineBridge::setParameter(const QString& id, const QString& v) { ntw_set_parameter_string(id.toUtf8().constData(), v.toUtf8().constData()); }

void EngineBridge::retainFrames(const QString& id) {
    if (shareCounts_[id]++ == 0) ntw_start_texture_sharing(id.toUtf8().constData());
}

void EngineBridge::releaseFrames(const QString& id) {
    auto it = shareCounts_.find(id);
    if (it == shareCounts_.end()) return;
    if (--it.value() > 0) return;
    shareCounts_.erase(it);
    if (running_) ntw_stop_texture_sharing(id.toUtf8().constData());
}

FrameExchange* EngineBridge::frames(const QString& id) const {
    return static_cast<FrameExchange*>(ntw_get_connectable_texture(id.toUtf8().constData()));
}

QString EngineBridge::shaderFilePath(const QString& shaderId) const {
    return running_ ? takeString(ntw_get_shader_file_path(shaderId.toUtf8().constData())) : QString();
}
