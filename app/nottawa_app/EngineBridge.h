#pragma once

#include <QHash>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QStringList>
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

namespace VideoSourceKind { enum { Webcam = 0, Image = 2, Icon = 3, Text = 5 }; }

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
    void    clearGraph();
    bool    loadWorkspace(const QString& path);
    void    saveWorkspace(const QString& path, const QString& name);

    QVector<ParamInfo> parameters(const QString& connectableId) const;
    void setParameter(const QString& paramId, float value);
    void setParameter(const QString& paramId, int value);
    void setParameter(const QString& paramId, bool value);
    void setParameter(const QString& paramId, const QString& value);

    QVector<StrandItem> strands() const;
    QString     strandsFolder() const;
    // Saves the chain ending at `nodeId` as a strand file; `previewPath` is
    // an optional thumbnail already written to disk.
    bool        saveStrand(const QString& nodeId, const QString& path, const QString& name,
                           const QString& previewPath = {});
    // Adds a saved strand's nodes to the graph; returns their ids.
    QStringList loadStrand(const QString& strandId);
    void        deleteStrand(const QString& strandId);
    void        renameStrand(const QString& strandId, const QString& name);

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

private:
    EngineBridge() = default;
    std::atomic<bool> running_{false};  // read by preview worker threads
    QHash<QString, int> shareCounts_;
};
