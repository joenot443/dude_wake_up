#pragma once

#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVector>

class FrameExchange;

struct CatalogItem {
    QString name;
    QString category;
    int     typeRaw  = 0;
    bool    isSource = false;
};

struct GraphNode {
    QString id;
    QString name;
    bool    isShader = false;
};

struct GraphEdge {
    QString startId;
    QString endId;
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
    QVector<GraphNode>   nodes() const;
    QVector<GraphEdge>   edges() const;
    // Deepest node with no outgoing connection, preferring shaders — the same
    // rule as NodeEditorViewModel's default stage output.
    QString outputNodeId() const;

    QString addEffect(int typeRaw, const QString& connectFromId);
    QString addSource(int typeRaw);
    void    clearGraph();
    bool    loadWorkspace(const QString& path);
    void    saveWorkspace(const QString& path, const QString& name);

    QVector<ParamInfo> parameters(const QString& connectableId) const;
    void setParameter(const QString& paramId, float value);
    void setParameter(const QString& paramId, int value);
    void setParameter(const QString& paramId, bool value);
    void setParameter(const QString& paramId, const QString& value);

    void startSharing(const QString& connectableId);
    void stopSharing(const QString& connectableId);
    // Null until the engine has produced the first shared frame.
    FrameExchange* frames(const QString& connectableId) const;

signals:
    void graphChanged();

private:
    EngineBridge() = default;
    bool running_ = false;
};
