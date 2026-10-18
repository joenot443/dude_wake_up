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

void onStrandsChanged() {
    QMetaObject::invokeMethod(EngineBridge::instance(), &EngineBridge::strandsChanged, Qt::QueuedConnection);
}

const char* const kEffectCategories[] = {"Basic", "Mix", "Transform", "Filter", "Mask", "Glitch"};

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
    ntw_engine_start();
    running_ = true;
    return true;
}

void EngineBridge::stop() {
    if (!running_) return;
    ntw_engine_stop();
    running_ = false;
}

float EngineBridge::fps() const { return running_ ? ntw_engine_fps() : 0.f; }

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
    case VideoSourceKind::Image: {
        if (path.isEmpty()) return {};
        const QByteArray name = QFileInfo(path).fileName().toUtf8();
        return takeString(ntw_add_image_video_source(name.constData(), path.toUtf8().constData()));
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
                       ntw_get_connectable_active(id.constData()), ntw_supports_aux_output(id.constData()),
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

QVector<StrandItem> EngineBridge::strands() const {
    QVector<StrandItem> out;
    int n = 0;
    NTWAvailableStrandInfo* arr = ntw_get_available_strands(&n);
    for (int i = 0; i < n; ++i) out.push_back({str(arr[i].id), str(arr[i].name), str(arr[i].imagePath)});
    ntw_free_available_strand_info_array(arr, n);
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

void EngineBridge::voteOnStrand(const QString& slug, const QString& voterId, const QString& vote) {
    { std::lock_guard<std::mutex> lk(g_communityMutex); g_voteSlugs.push_back(slug); }
    ntw_vote_strand(slug.toUtf8().constData(), voterId.toUtf8().constData(), vote.toUtf8().constData(), &onVote);
}

void EngineBridge::fetchSharedStrand(const QString& slug) {
    { std::lock_guard<std::mutex> lk(g_communityMutex); g_fetchSlugs.push_back(slug); }
    ntw_fetch_shared_strand(slug.toUtf8().constData(), &onFetchStrand);
}

void EngineBridge::shareStrand(const QString& nodeId, const QString& title, const QString& description,
                               const QString& author, const QByteArray& previewPng) {
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
bool EngineBridge::loadWorkspace(const QString& path) { return ntw_load_workspace(path.toUtf8().constData()); }

void EngineBridge::saveWorkspace(const QString& path, const QString& name) {
    ntw_save_workspace_to_path(path.toUtf8().constData(), name.toUtf8().constData());
}

QVector<ParamInfo> EngineBridge::parameters(const QString& connectableId) const {
    QVector<ParamInfo> out;
    int n = 0;
    NTWParameterInfo* arr = ntw_get_parameters(connectableId.toUtf8().constData(), &n);
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
        out.push_back(info);
    }
    ntw_free_parameter_info_array(arr, n);
    return out;
}

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
