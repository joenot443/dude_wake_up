#include "InspectorPanel.h"

#include "Icons.h"
#include "StageWidget.h"
#include "Theme.h"
#include "inspector/ParameterRow.h"
#include "inspector/PositionPad.h"
#include "ui/Controls.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace {

constexpr int kShaderFeedback = 7;   // ShaderType raw values
constexpr int kShaderBounce   = 58;

QFrame* divider(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1; border: none;").arg(Theme::border.name()));
    return line;
}

QToolButton* headerButton(Icons::Kind icon, const QString& tip, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setIcon(Icons::icon(icon, Theme::textSecondary));
    b->setIconSize(QSize(14, 14));
    b->setAutoRaise(true);
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QPushButton* actionButton(const QString& text, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QPushButton { background: transparent; color: %1; border: 1px solid %2;"
                             " border-radius: 6px; padding: 5px; font-size: 8pt; }"
                             "QPushButton:hover { background: %3; }")
                         .arg(Theme::accent.name(), Theme::accent.darker(150).name(), Theme::accentSubtle.name()));
    return b;
}

// The macOS rules for an X/Y pair shown as a position pad: plain X/Y and
// "X/Y Position" in a 0..1 range, and the orb of Electric Eels.
QString pairedYName(const ParamInfo& p) {
    if (p.name == "Orb X") return "Orb Y";
    const bool unit = p.min >= 0 && p.max <= 1.0f;
    if (!unit) return {};
    if (p.name == "X") return "Y";
    if (p.name == "X Position") return "Y Position";
    return {};
}

// Calls a function when the watched widget is clicked.
class ClickForwarder : public QObject {
public:
    ClickForwarder(QObject* parent, std::function<void()> onClick) : QObject(parent), onClick_(std::move(onClick)) {}
    bool eventFilter(QObject*, QEvent* e) override {
        if (e->type() == QEvent::MouseButtonRelease) onClick_();
        return false;
    }
private:
    std::function<void()> onClick_;
};

// A 16:9 live preview.
class AspectPreview : public StageWidget {
public:
    AspectPreview(QWidget* parent) : StageWidget(parent, FramePresenter::Inspector) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int w) const override { return w * 9 / 16; }
    QSize sizeHint() const override { return QSize(288, 162); }
};

} // anon

// ------------------------------------------------------- optional effects

// The node's built-in optional effects: a switch each, and when on, a
// chevron that expands its parameters.
class OptionalEffectsSection : public QWidget {
public:
    OptionalEffectsSection(const QString& nodeId, QWidget* parent) : QWidget(parent), nodeId_(nodeId) {
        layout_ = new QVBoxLayout(this);
        layout_->setContentsMargins(0, 0, 0, 0);
        layout_->setSpacing(6);
        rebuild();
    }

    void refresh() {
        const QVector<OptionalEffect> effects = EngineBridge::instance()->optionalEffects(nodeId_);
        bool same = effects.size() == effects_.size();
        for (int i = 0; same && i < effects.size(); ++i) same = effects[i].enabled == effects_[i].enabled;
        if (!same) { rebuild(); return; }
        if (expanded_ < 0) return;
        for (const ParamInfo& p : EngineBridge::instance()->optionalEffectParameters(nodeId_, expanded_))
            if (ParameterRow* row = rows_.value(p.id)) row->setInfo(p);
    }

private:
    void rebuild() {
        while (QLayoutItem* item = layout_->takeAt(0)) {
            if (QWidget* w = item->widget()) w->deleteLater();
            else if (QLayout* l = item->layout()) {
                while (QLayoutItem* inner = l->takeAt(0)) { if (inner->widget()) inner->widget()->deleteLater(); delete inner; }
            }
            delete item;
        }
        rows_.clear();
        effects_ = EngineBridge::instance()->optionalEffects(nodeId_);
        if (effects_.isEmpty()) return;

        layout_->addWidget(divider(this));
        auto* header = new QHBoxLayout;
        auto* title = new QLabel("Optional Effects", this);
        title->setStyleSheet(QString("color: %1; font-weight: 600;").arg(Theme::textSecondary.name()));
        header->addWidget(title);
        header->addStretch(1);
        if (std::any_of(effects_.begin(), effects_.end(), [](const OptionalEffect& e) { return e.enabled; })) {
            auto* reset = headerButton(Icons::Reset, "Disable all optional effects", this);
            reset->setIconSize(QSize(11, 11));
            connect(reset, &QToolButton::clicked, this, [this] {
                for (const OptionalEffect& e : effects_)
                    if (e.enabled) EngineBridge::instance()->toggleOptionalEffect(nodeId_, e.index);
                expanded_ = -1;
                rebuild();
            });
            header->addWidget(reset);
        }
        layout_->addLayout(header);

        for (const OptionalEffect& effect : effects_) {
            auto* row = new QHBoxLayout;
            auto* toggle = new DSToggle(effect.name, this);
            toggle->setSmall(true);
            toggle->setChecked(effect.enabled);
            const int index = effect.index;
            connect(toggle, &DSToggle::toggled, this, [this, index](bool on) {
                EngineBridge::instance()->toggleOptionalEffect(nodeId_, index);
                // Enabling expands its parameters; disabling collapses them.
                expanded_ = on ? index : (expanded_ == index ? -1 : expanded_);
                QMetaObject::invokeMethod(this, [this] { rebuild(); }, Qt::QueuedConnection);
            });
            row->addWidget(toggle);
            row->addStretch(1);
            if (effect.enabled) {
                auto* chevron = headerButton(expanded_ == index ? Icons::ChevronUp : Icons::ChevronDown, "Parameters", this);
                chevron->setIconSize(QSize(10, 10));
                connect(chevron, &QToolButton::clicked, this, [this, index] {
                    expanded_ = expanded_ == index ? -1 : index;
                    QMetaObject::invokeMethod(this, [this] { rebuild(); }, Qt::QueuedConnection);
                });
                row->addWidget(chevron);
            }
            layout_->addLayout(row);
            if (effect.enabled && expanded_ == index) {
                for (const ParamInfo& p : EngineBridge::instance()->optionalEffectParameters(nodeId_, index)) {
                    if (p.type == ParamInfo::Hidden || p.hidden) continue;
                    auto* paramRow = new ParameterRow(p, ParameterRow::Full, {}, this);
                    paramRow->setContentsMargins(8, 0, 0, 0);
                    rows_.insert(p.id, paramRow);
                    layout_->addWidget(paramRow);
                }
            }
        }
    }

    QString                 nodeId_;
    QVBoxLayout*            layout_;
    QVector<OptionalEffect> effects_;
    QHash<QString, ParameterRow*> rows_;
    int                     expanded_ = -1;
};

// -------------------------------------------------------------- inspector

InspectorPanel::InspectorPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("InspectorPanel");
    setStyleSheet(QString("#InspectorPanel, #InspectorContent { background: %1; }"
                          "#InspectorPanel QLineEdit { background: %2; border: 1px solid %3; border-radius: 5px;"
                          " padding: 3px 6px; color: %4; }")
                      .arg(Theme::backgroundSecondary.name(), Theme::backgroundTertiary.name(), Theme::border.name(),
                           Theme::textPrimary.name()));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    header_ = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(header_);
    headerLayout->setContentsMargins(16, 12, 12, 12);
    headerLayout->setSpacing(8);
    pip_ = new QLabel(header_);
    title_ = new QLabel(header_);
    title_->setStyleSheet(QString("color: %1; font-size: 11pt; font-weight: 600;").arg(Theme::textPrimary.name()));
    resetAll_ = headerButton(Icons::Reset, "Reset all parameters", header_);
    connect(resetAll_, &QToolButton::clicked, this, [this] {
        EngineBridge::instance()->resetAllParameters(nodeId_);
        emit graphMayChange();
        refresh();
    });
    auto* close = headerButton(Icons::Close, "Deselect", header_);
    connect(close, &QToolButton::clicked, this, &InspectorPanel::deselectRequested);
    headerLayout->addWidget(pip_);
    headerLayout->addWidget(title_, 1);
    headerLayout->addWidget(resetAll_);
    headerLayout->addWidget(close);
    layout->addWidget(header_);
    layout->addWidget(divider(this));

    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    content_ = new QWidget(scroll_);
    content_->setObjectName("InspectorContent");
    // Track the viewport's width; rows lay out within it rather than
    // pushing the content wider than the panel.
    content_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setContentsMargins(0, 0, 0, 0);
    contentLayout_->setSpacing(0);
    scroll_->setWidget(content_);
    layout->addWidget(scroll_, 1);

    empty_ = new QLabel("Select a node to see its parameters", this);
    empty_->setAlignment(Qt::AlignCenter);
    empty_->setWordWrap(true);
    empty_->setStyleSheet(QString("color: %1; padding: 24px;").arg(Theme::textTertiary.name()));
    layout->addWidget(empty_, 1);

    refreshTimer_.setInterval(100);
    connect(&refreshTimer_, &QTimer::timeout, this, &InspectorPanel::refresh);
    setSelection({});
}

void InspectorPanel::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    if (!nodeId_.isEmpty()) refreshTimer_.start();
}

void InspectorPanel::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    refreshTimer_.stop();
}

void InspectorPanel::clearContent() {
    while (QLayoutItem* item = contentLayout_->takeAt(0)) {
        if (QWidget* w = item->widget()) delete w;
        delete item;
    }
    preview_ = nullptr;
    params_ = nullptr;
    paramsLayout_ = nullptr;
    optional_ = nullptr;
    rows_.clear();
    pads_.clear();
    signature_.clear();
}

void InspectorPanel::setSelection(const QStringList& ids) {
    QVector<GraphNode> nodes;
    if (!ids.isEmpty())
        for (const GraphNode& n : EngineBridge::instance()->nodes())
            if (ids.contains(n.id)) nodes.push_back(n);
    // The same single node: keep everything (and its expanded panels).
    if (nodes.size() == 1 && nodes.first().id == nodeId_ && selection_ == ids) {
        if (nodes.first().name != node_.name) { node_ = nodes.first(); title_->setText(node_.name); }
        return;
    }
    selection_ = ids;
    clearContent();
    nodeId_.clear();
    refreshTimer_.stop();
    if (nodes.isEmpty()) {
        header_->hide();
        scroll_->hide();
        empty_->show();
        return;
    }
    header_->show();
    scroll_->show();
    empty_->hide();
    if (nodes.size() == 1) showNode(nodes.first());
    else showMultiple(nodes);
}

void InspectorPanel::showMultiple(const QVector<GraphNode>& nodes) {
    pip_->setPixmap(Icons::icon(Icons::Stack, Theme::accent).pixmap(14, 14));
    title_->setText(QString("%1 nodes selected").arg(nodes.size()));
    resetAll_->hide();
    auto* list = new QWidget(content_);
    auto* listLayout = new QVBoxLayout(list);
    listLayout->setContentsMargins(16, 16, 16, 16);
    listLayout->setSpacing(4);
    for (const GraphNode& n : nodes) {
        auto* row = new QLabel(QString("<span style='color:%1'>●</span>&nbsp; %2").arg(Theme::accent.name(), n.name.toHtmlEscaped()), list);
        row->setStyleSheet(QString("QLabel { background: %1; border-radius: 5px; padding: 5px 10px; color: %2; }"
                                   "QLabel:hover { background: %3; }")
                               .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::surfaceHover.name()));
        row->setCursor(Qt::PointingHandCursor);
        row->setToolTip(n.isShader ? "Effect" : "Source");
        row->installEventFilter(new ClickForwarder(row, [this, id = n.id] { emit nodeRequested(id); }));
        listLayout->addWidget(row);
    }
    listLayout->addStretch(1);
    contentLayout_->addWidget(list);
}

void InspectorPanel::showNode(const GraphNode& node) {
    node_ = node;
    nodeId_ = node.id;
    QPixmap dot(16, 16);
    dot.fill(Qt::transparent);
    {
        QPainter p(&dot);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(node.isShader ? Theme::accent : Theme::success);
        p.drawEllipse(QRectF(4, 4, 8, 8));
    }
    pip_->setPixmap(dot);
    title_->setText(node.name);
    resetAll_->show();

    auto* previewBox = new QWidget(content_);
    auto* previewLayout = new QVBoxLayout(previewBox);
    previewLayout->setContentsMargins(16, 8, 16, 8);
    preview_ = new AspectPreview(previewBox);
    preview_->setSourceNode(nodeId_);
    previewLayout->addWidget(preview_);
    contentLayout_->addWidget(previewBox);
    contentLayout_->addWidget(divider(content_));

    params_ = new QWidget(content_);
    paramsLayout_ = new QVBoxLayout(params_);
    paramsLayout_->setContentsMargins(10, 10, 10, 10);
    paramsLayout_->setSpacing(4);
    contentLayout_->addWidget(params_);

    auto* actions = new QWidget(content_);
    auto* actionsLayout = new QVBoxLayout(actions);
    actionsLayout->setContentsMargins(16, 4, 16, 4);
    actionsLayout->setSpacing(8);
    if (node.isShader && node.typeRaw == kShaderFeedback) {
        auto* b = actionButton("Clear Feedback Buffer", actions);
        connect(b, &QPushButton::clicked, this, [this] { EngineBridge::instance()->clearFeedbackBuffer(nodeId_); });
        actionsLayout->addWidget(b);
    }
    if (node.isShader && node.typeRaw == kShaderBounce) {
        auto* b = actionButton("Reset Position", actions);
        connect(b, &QPushButton::clicked, this, [this] { EngineBridge::instance()->resetBouncePosition(nodeId_); });
        actionsLayout->addWidget(b);
    }
    contentLayout_->addWidget(actions);

    auto* optionalBox = new QWidget(content_);
    auto* optionalLayout = new QVBoxLayout(optionalBox);
    optionalLayout->setContentsMargins(16, 4, 16, 16);
    optional_ = new OptionalEffectsSection(nodeId_, optionalBox);
    optionalLayout->addWidget(optional_);
    contentLayout_->addWidget(optionalBox);
    contentLayout_->addStretch(1);

    rebuildParameters(EngineBridge::instance()->parameters(nodeId_));
    if (isVisible()) refreshTimer_.start();
}

QVector<ParamInfo> InspectorPanel::visibleParameters(const QVector<ParamInfo>& all) const {
    static const QStringList kPlumbing = {"width", "height", "start", "end"};  // VideoSourceSettings internals
    QStringList pairedY;
    for (const ParamInfo& p : all) {
        const QString y = pairedYName(p);
        if (y.isEmpty()) continue;
        for (const ParamInfo& q : all)
            if (q.name == y && q.type != ParamInfo::Hidden && (y == "Orb Y" || (q.min >= 0 && q.max <= 1.0f)))
                pairedY << q.id;
    }
    QVector<ParamInfo> out;
    for (const ParamInfo& p : all) {
        if (p.type == ParamInfo::Hidden || p.hidden || kPlumbing.contains(p.name) || pairedY.contains(p.id)) continue;
        out.push_back(p);
    }
    return out;
}

QString InspectorPanel::signature(const QVector<ParamInfo>& visible) const {
    QString s;
    for (const ParamInfo& p : visible)
        s += QString("%1/%2/%3/%4;").arg(p.id).arg(p.type).arg(p.options.size()).arg(p.hasOscillator);
    return s;
}

void InspectorPanel::rebuildParameters(const QVector<ParamInfo>& all) {
    while (QLayoutItem* item = paramsLayout_->takeAt(0)) {
        if (QWidget* w = item->widget()) delete w;
        delete item;
    }
    rows_.clear();
    pads_.clear();
    const QVector<ParamInfo> visible = visibleParameters(all);
    signature_ = signature(visible);

    for (const ParamInfo& p : visible) {
        const QString yName = pairedYName(p);
        const ParamInfo* y = nullptr;
        if (!yName.isEmpty())
            for (const ParamInfo& q : all) if (q.name == yName && q.type != ParamInfo::Hidden) y = &q;
        if (!y) {
            auto* row = new ParameterRow(p, ParameterRow::Full, {}, params_);
            connect(row, &ParameterRow::graphMayChange, this, &InspectorPanel::graphMayChange);
            rows_.insert(p.id, row);
            paramsLayout_->addWidget(row);
            continue;
        }
        // An X/Y pair: a pad plus reactivity rows per axis.
        auto* box = new QWidget(params_);
        auto* boxLayout = new QVBoxLayout(box);
        boxLayout->setContentsMargins(6, 4, 6, 4);
        boxLayout->setSpacing(4);
        auto* label = new QLabel("Position", box);
        label->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
        boxLayout->addWidget(label);
        PadGroup group;
        group.x = p;
        group.y = *y;
        group.invertY = y->name != "Orb Y";  // the orb is in screen space already
        group.pad = new PositionPad(nodeId_, box);
        boxLayout->addWidget(group.pad);
        group.xRow = new ParameterRow(p, ParameterRow::Axis, "X", box);
        group.yRow = new ParameterRow(*y, ParameterRow::Axis, "Y", box);
        boxLayout->addWidget(group.xRow);
        boxLayout->addWidget(group.yRow);
        const int index = int(pads_.size());
        connect(group.pad, &PositionPad::moved, this, [this, index](float nx, float ny) {
            const PadGroup& g = pads_[index];
            auto denorm = [](float n, const ParamInfo& q) { return q.min + n * (q.max - q.min); };
            EngineBridge::instance()->setParameter(g.x.id, denorm(nx, g.x));
            EngineBridge::instance()->setParameter(g.y.id, denorm(g.invertY ? 1.0f - ny : ny, g.y));
        });
        pads_.push_back(group);
        paramsLayout_->addWidget(box);
    }
    refresh();
}

void InspectorPanel::refresh() {
    if (nodeId_.isEmpty() || !paramsLayout_) return;
    const QVector<ParamInfo> all = EngineBridge::instance()->parameters(nodeId_);
    if (all.isEmpty() && !rows_.isEmpty()) return;  // node going away
    if (signature(visibleParameters(all)) != signature_) {
        rebuildParameters(all);
        return;
    }
    QHash<QString, const ParamInfo*> byId;
    for (const ParamInfo& p : all) byId.insert(p.id, &p);
    for (auto it = rows_.begin(); it != rows_.end(); ++it)
        if (const ParamInfo* p = byId.value(it.key())) it.value()->setInfo(*p);
    for (PadGroup& g : pads_) {
        const ParamInfo* x = byId.value(g.x.id);
        const ParamInfo* y = byId.value(g.y.id);
        if (!x || !y) continue;
        g.x = *x;
        g.y = *y;
        auto norm = [](const ParamInfo& q) { return q.max != q.min ? (q.value - q.min) / (q.max - q.min) : 0.0f; };
        g.pad->setPosition(norm(*x), g.invertY ? 1.0f - norm(*y) : norm(*y));
        g.xRow->setInfo(*x);
        g.yRow->setInfo(*y);
    }
    if (optional_) optional_->refresh();
}
