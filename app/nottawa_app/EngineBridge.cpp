#include "EngineBridge.h"

#include "nottawa_bridge.h"

#include <QHash>
#include <QSet>
#include <cstdlib>

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

const char* const kEffectCategories[] = {"Basic", "Mix", "Transform", "Filter", "Mask", "Glitch"};

} // anon

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
            out.push_back({str(arr[i].name), kEffectCategories[c - 1], arr[i].shaderTypeRaw, false});
        ntw_free_available_shader_info_array(arr, n);
    }
    return out;
}

QVector<CatalogItem> EngineBridge::sources() const {
    QVector<CatalogItem> out;
    int n = 0;
    NTWAvailableShaderInfo* arr = ntw_get_available_shader_sources(&n);
    for (int i = 0; i < n; ++i)
        out.push_back({str(arr[i].name), str(arr[i].category), arr[i].shaderTypeRaw, true});
    ntw_free_available_shader_info_array(arr, n);
    return out;
}

QVector<GraphNode> EngineBridge::nodes() const {
    QVector<GraphNode> out;
    int n = 0;
    NTWConnectableInfo* shaders = ntw_get_all_shaders(&n);
    for (int i = 0; i < n; ++i) {
        const auto& s = shaders[i];
        const QByteArray id(s.id);
        out.push_back({str(s.id), str(s.name), true, s.shaderTypeRaw, s.inputCount, QPointF(s.posX, s.posY),
                       ntw_get_connectable_active(id.constData()), ntw_supports_aux_output(id.constData())});
    }
    ntw_free_connectable_info_array(shaders, n);
    NTWVideoSourceInfo* srcs = ntw_get_all_video_sources(&n);
    for (int i = 0; i < n; ++i) {
        const auto& s = srcs[i];
        out.push_back({str(s.id), str(s.name), false, s.sourceType, s.inputCount, QPointF(s.posX, s.posY),
                       ntw_get_connectable_active(s.id), false});
    }
    ntw_free_video_source_info_array(srcs, n);
    return out;
}

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
