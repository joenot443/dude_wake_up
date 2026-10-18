#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QStringList>
#include <QTimer>
#include <QWidget>

class FileSourcePanel;
class IconSourcePanel;
class OptionalEffectsSection;
class AuxStrandSection;
class TextSourcePanel;
class TransformSection;
class ParameterRow;
class PositionPad;
class QLabel;
class QScrollArea;
class QToolButton;
class QVBoxLayout;
class StageWidget;

// The right sidebar, after the macOS ParameterInspectorView. One node
// selected: its name, a live preview, its parameters (with oscillator,
// audio-driver and MIDI controls, position pads for X/Y pairs), node
// actions and the optional effects. Several selected: a list of them.
// Values refresh ten times a second (oscillators and audio move them).
class InspectorPanel : public QWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);
    void setSelection(const QStringList& ids);
    QString node() const { return nodeId_; }
    // A parameter row's rect in global coordinates (null if not shown).
    QRect parameterRowRect(const QString& paramId) const;

signals:
    void deselectRequested();
    void nodeRequested(const QString& id);
    // A parameter change that can alter the graph (e.g. adds a pin).
    void graphMayChange();
    void editShaderRequested(const QString& shaderId, const QString& name);

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    struct PadGroup {
        PositionPad*  pad = nullptr;
        ParameterRow* xRow = nullptr;
        ParameterRow* yRow = nullptr;
        ParamInfo     x, y;
        bool          invertY = true;
    };

    void showNode(const GraphNode& node);
    void showMultiple(const QVector<GraphNode>& nodes);
    void clearContent();
    void rebuildParameters(const QVector<ParamInfo>& params);
    void refresh();
    // Visible parameters in order; Y params of position pairs are dropped.
    QVector<ParamInfo> visibleParameters(const QVector<ParamInfo>& all) const;
    QString signature(const QVector<ParamInfo>& visible) const;

    QString      nodeId_;
    GraphNode    node_;
    QStringList  selection_;
    QWidget*     header_ = nullptr;
    QLabel*      pip_ = nullptr;
    QLabel*      title_ = nullptr;
    QToolButton* resetAll_ = nullptr;
    QScrollArea* scroll_ = nullptr;
    QWidget*     content_ = nullptr;
    QVBoxLayout* contentLayout_ = nullptr;
    QLabel*      empty_ = nullptr;

    StageWidget* preview_ = nullptr;
    QWidget*     params_ = nullptr;
    QVBoxLayout* paramsLayout_ = nullptr;
    QHash<QString, ParameterRow*> rows_;
    QVector<PadGroup> pads_;
    OptionalEffectsSection* optional_ = nullptr;
    TextSourcePanel* textPanel_ = nullptr;   // text sources
    IconSourcePanel* iconPanel_ = nullptr;   // icon sources
    FileSourcePanel* filePanel_ = nullptr;   // video file / library sources
    TransformSection* transform_ = nullptr;  // crop / place pads
    AuxStrandSection* aux_ = nullptr;        // nodes with an aux output
    QLabel*      modPill_ = nullptr;         // shader has a session edit
    QWidget*     maskPresets_ = nullptr;     // the Mask shader's presets
    QString      signature_;
    QTimer       refreshTimer_;
};
