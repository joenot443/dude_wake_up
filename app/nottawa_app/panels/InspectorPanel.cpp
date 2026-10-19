#include "InspectorPanel.h"

#include "panels/CatalogPopup.h"

#include "Icons.h"
#include "StageWidget.h"
#include "Theme.h"
#include "inspector/ParameterRow.h"
#include "inspector/PositionPad.h"
#include "inspector/SourcePanels.h"
#include "inspector/TransformPads.h"
#include "ui/Controls.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QSet>
#include <QMenu>
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
    // Without this the layout treats the preferred width as the minimum
    // and the panel's content can't narrow to fit.
    QSize minimumSizeHint() const override { return QSize(64, 36); }
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

// ------------------------------------------------------------- transform

// Crop and Place pads for nodes with the SourceTransform pipeline (the
// Transform shader, webcams), replacing their crop/placement sliders.
class TransformSection : public QWidget {
public:
    static bool applies(const QVector<ParamInfo>& params) {
        static const QStringList required = {"Min X", "Max X", "Min Y", "Max Y", "X", "Y", "Scale", "Mode"};
        for (const QString& name : required)
            if (std::none_of(params.begin(), params.end(), [&](const ParamInfo& p) { return p.name == name; })) return false;
        return true;
    }

    TransformSection(const GraphNode& node, QWidget* parent) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(6, 4, 6, 8);
        layout->setSpacing(6);
        layout->addLayout(header("Crop", "Reset crop to full frame", [this] { setParams({{"Min X", 0}, {"Max X", 1}, {"Min Y", 0}, {"Max Y", 1}}); }));
        crop_ = new CropPad(this);
        // Sources expose their pre-transform frame under "sourceRaw:<id>";
        // effects crop whatever feeds their main input.
        QString input = node.isShader ? QString() : "sourceRaw:" + node.id;
        if (node.isShader)
            for (const GraphEdge& e : EngineBridge::instance()->edges())
                if (e.endId == node.id && e.inputSlot == 0) input = e.startId;
        crop_->setFrameSource(input);
        connect(crop_, &CropPad::cropChanged, this, [this](float x0, float x1, float y0, float y1) {
            setParams({{"Min X", x0}, {"Max X", x1}, {"Min Y", y0}, {"Max Y", y1}});
            cropLabel_->setText(cropText(x0, x1, y0, y1));
        });
        layout->addWidget(crop_);
        cropLabel_ = caption(this);
        layout->addWidget(cropLabel_);

        placeBox_ = new QWidget(this);
        auto* placeLayout = new QVBoxLayout(placeBox_);
        placeLayout->setContentsMargins(0, 6, 0, 0);
        placeLayout->setSpacing(6);
        placeLayout->addLayout(header("Place", "Reset position and scale", [this] { setParams({{"X", 0}, {"Y", 0}, {"Scale", 1}}); }));
        place_ = new PlacePad(placeBox_);
        place_->setFrameSource("transformCrop:" + node.id);
        connect(place_, &PlacePad::placementChanged, this, [this](float x, float y, float s) {
            setParams({{"X", x}, {"Y", y}, {"Scale", s}});
            placeLabel_->setText(placeText(x, y, s));
        });
        placeLayout->addWidget(place_);
        placeLabel_ = caption(placeBox_);
        placeLayout->addWidget(placeLabel_);
        layout->addWidget(placeBox_);
    }

    void setParams(const QVector<ParamInfo>& params) {
        params_ = params;
        auto v = [&](const char* name) {
            for (const ParamInfo& p : params) if (p.name == name) return p.type == ParamInfo::Int ? float(p.intValue) : p.value;
            return 0.0f;
        };
        const float x0 = v("Min X"), x1 = v("Max X"), y0 = v("Min Y"), y1 = v("Max Y");
        crop_->setCrop(x0, x1, y0, y1);
        if (!crop_->isDragging()) cropLabel_->setText(cropText(x0, x1, y0, y1));
        // Placement only applies in Standard mode; the auto-fit modes
        // compute it.
        placeBox_->setVisible(int(v("Mode")) == 0);
        place_->setPlacement(v("X"), v("Y"), v("Scale"), std::max(0.0f, x1 - x0), std::max(0.0f, y1 - y0));
        if (!place_->isDragging()) placeLabel_->setText(placeText(v("X"), v("Y"), v("Scale")));
    }

private:
    QHBoxLayout* header(const QString& title, const QString& resetTip, std::function<void()> onReset) {
        auto* row = new QHBoxLayout;
        auto* label = new QLabel(title, this);
        label->setStyleSheet(QString("color: %1; font-weight: 600;").arg(Theme::textSecondary.name()));
        auto* reset = headerButton(Icons::Reset, resetTip, this);
        reset->setIconSize(QSize(11, 11));
        connect(reset, &QToolButton::clicked, this, std::move(onReset));
        row->addWidget(label);
        row->addStretch(1);
        row->addWidget(reset);
        return row;
    }
    static QLabel* caption(QWidget* parent) {
        auto* l = new QLabel(parent);
        l->setStyleSheet(QString("color: %1; font-size: 8pt; font-family: Consolas;").arg(Theme::textTertiary.name()));
        return l;
    }
    static QString cropText(float x0, float x1, float y0, float y1) {
        return QString("X %1 – %2      Y %3 – %4").arg(x0, 0, 'f', 2).arg(x1, 0, 'f', 2).arg(y0, 0, 'f', 2).arg(y1, 0, 'f', 2);
    }
    static QString placeText(float x, float y, float s) {
        return QString("X %1  Y %2      Scale %3").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2).arg(s, 0, 'f', 2);
    }
    void setParams(std::initializer_list<std::pair<const char*, float>> values) {
        for (const auto& [name, value] : values)
            for (const ParamInfo& p : params_)
                if (p.name == name) EngineBridge::instance()->setParameter(p.id, value);
    }

    QVector<ParamInfo> params_;
    CropPad*  crop_ = nullptr;
    PlacePad* place_ = nullptr;
    QWidget*  placeBox_ = nullptr;
    QLabel*   cropLabel_ = nullptr;
    QLabel*   placeLabel_ = nullptr;
};

// ------------------------------------------------------------ aux strand

// The chain processing a node's auxiliary output (Feedback and the mask
// makers), after the macOS FeedbackAuxStrandView: chips for its nodes
// (click selects, × disconnects) and a button that appends a filter.
class AuxStrandSection : public QWidget {
public:
    AuxStrandSection(InspectorPanel* panel, const GraphNode& node, QWidget* parent)
        : QWidget(parent), panel_(panel), node_(node) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(6);
        layout->addWidget(divider(this));
        auto* title = new QLabel("Auxiliary Strand", this);
        title->setStyleSheet(QString("color: %1; font-weight: 600;").arg(Theme::textSecondary.name()));
        layout->addWidget(title);
        auto* blurb = new QLabel("Each feedback frame is processed through this chain before mixing back in.", this);
        blurb->setWordWrap(true);
        blurb->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textTertiary.name()));
        layout->addWidget(blurb);
        chips_ = new QWidget(this);
        chipLayout_ = new QHBoxLayout(chips_);
        chipLayout_->setContentsMargins(0, 2, 0, 2);
        chipLayout_->setSpacing(6);
        layout->addWidget(chips_);
        refresh();
    }

    // Rebuilds the chips when the chain changed.
    void refresh() {
        const QVector<GraphEdge> edges = EngineBridge::instance()->edges();
        QVector<GraphNode> nodes = EngineBridge::instance()->nodes();
        // First link from the aux output (slot 10), then main outputs.
        QVector<GraphNode> chain;
        QVector<QString> incoming;  // connection anchoring each chain node
        QSet<QString> seen{node_.id};
        QString current = node_.id;
        int slot = kAuxOutputSlot;
        for (;;) {
            const GraphEdge* next = nullptr;
            for (const GraphEdge& e : edges) if (e.startId == current && e.outputSlot == slot) { next = &e; break; }
            if (!next || seen.contains(next->endId)) break;
            const auto it = std::find_if(nodes.begin(), nodes.end(), [&](const GraphNode& n) { return n.id == next->endId; });
            if (it == nodes.end()) break;
            chain.push_back(*it);
            incoming.push_back(next->id);
            seen.insert(next->endId);
            current = next->endId;
            slot = 0;
        }
        QString signature;
        for (const GraphNode& n : chain) signature += n.id + n.name + ';';
        if (signature == signature_ && chipLayout_->count() > 0) return;
        signature_ = signature;
        chain_ = chain;

        while (QLayoutItem* item = chipLayout_->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        const QString chipStyle = QString("QToolButton { background: %1; color: %2; border: 1px solid %3; border-radius: 10px;"
                                          " padding: 2px 8px; font-size: 8pt; }"
                                          "QToolButton:hover { background: %4; }")
                                      .arg(Theme::surface.name(), Theme::textPrimary.name(),
                                           Theme::warning.darker(140).name(), Theme::surfaceHover.name());
        for (int i = 0; i < chain.size(); ++i) {
            auto* chip = new QToolButton(chips_);
            chip->setText(chain[i].name + "  ×");
            chip->setToolTip("Click to select; × at the end removes it from the strand");
            chip->setStyleSheet(chipStyle);
            chip->setCursor(Qt::PointingHandCursor);
            const QString id = chain[i].id, connection = incoming[i];
            connect(chip, &QToolButton::clicked, this, [this, chip, id, connection] {
                // The trailing "×" disconnects; the rest of the chip selects.
                if (chip->mapFromGlobal(QCursor::pos()).x() > chip->width() - 18) {
                    EngineBridge::instance()->removeConnection(connection);
                    refresh();
                } else {
                    emit panel_->nodeRequested(id);
                }
            });
            chipLayout_->addWidget(chip);
            auto* arrow = new QLabel("→", chips_);
            arrow->setStyleSheet(QString("color: %1;").arg(Theme::textTertiary.name()));
            chipLayout_->addWidget(arrow);
        }
        auto* add = new QToolButton(chips_);
        add->setText(chain.isEmpty() ? "+ Add Filter" : "+ Add");
        add->setCursor(Qt::PointingHandCursor);
        add->setStyleSheet(QString("QToolButton { background: transparent; color: %1; border: 1px solid %2; border-radius: 10px;"
                                   " padding: 2px 8px; font-size: 8pt; }"
                                   "QToolButton:hover { background: %3; }")
                               .arg(Theme::accent.name(), Theme::accent.darker(140).name(), Theme::accentSubtle.name()));
        connect(add, &QToolButton::clicked, this, [this, add] {
            // The live-preview effect browser, as on macOS.
            auto* browser = new CatalogPopup(true, [this](int type) { addFilter(type); }, this);
            browser->popup(add->mapToGlobal(QPoint(0, add->height() + 4)));
        });
        chipLayout_->addWidget(add);
        chipLayout_->addStretch(1);
    }

private:
    void addFilter(int type) {
        auto* engine = EngineBridge::instance();
        const QString start = chain_.isEmpty() ? node_.id : chain_.last().id;
        QPointF anchor = node_.position;
        for (const GraphNode& n : engine->nodes()) if (n.id == start) anchor = n.position;
        const QString id = engine->addEffect(type, {});
        if (id.isEmpty()) return;
        engine->setNodePosition(id, anchor + QPointF(320, 0));
        engine->connectNodes(start, id, chain_.isEmpty() ? kAuxOutputSlot : 0, 0);
        refresh();
    }

    InspectorPanel* panel_;
    GraphNode    node_;
    QWidget*     chips_;
    QHBoxLayout* chipLayout_;
    QVector<GraphNode> chain_;
    QString      signature_;
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
    aux_ = nullptr;
    modPill_ = nullptr;
    maskPresets_ = nullptr;
    textPanel_ = nullptr;
    iconPanel_ = nullptr;
    filePanel_ = nullptr;
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

    // The Mask shader's presets set its Source Mode (and a matching Output
    // Mode); the shader then shows the parameters for that mode.
    constexpr int kShaderMaskEngine = 182;
    if (node.isShader && node.typeRaw == kShaderMaskEngine) {
        const QVector<ParamInfo> params = EngineBridge::instance()->parameters(nodeId_);
        QString sourceMode, outputMode;
        for (const ParamInfo& p : params) {
            if (p.name == "Source Mode") sourceMode = p.id;
            if (p.name == "Output Mode") outputMode = p.id;
        }
        if (!sourceMode.isEmpty()) {
            maskPresets_ = new QWidget(content_);
            auto* layout = new QVBoxLayout(maskPresets_);
            layout->setContentsMargins(16, 8, 16, 8);
            layout->setSpacing(6);
            auto* label = new QLabel("Preset", maskPresets_);
            label->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
            layout->addWidget(label);
            auto* row = new QHBoxLayout;
            row->setSpacing(6);
            const QStringList presets = {"Luma", "Color Key", "Match Blend"};
            for (int i = 0; i < presets.size(); ++i) {
                auto* b = new QPushButton(presets[i], maskPresets_);
                b->setCheckable(true);
                b->setCursor(Qt::PointingHandCursor);
                b->setProperty("preset", i);
                b->setStyleSheet(QString("QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 6px;"
                                         " padding: 4px; font-size: 8pt; }"
                                         "QPushButton:checked { background: %4; color: white; border-color: %4; }")
                                     .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::border.name(),
                                          Theme::accent.name()));
                connect(b, &QPushButton::clicked, this, [this, i, sourceMode, outputMode] {
                    EngineBridge::instance()->setParameter(sourceMode, i);
                    // Blend output suits Match Blend; the others output a mask.
                    if (!outputMode.isEmpty()) EngineBridge::instance()->setParameter(outputMode, i == 2 ? 1 : 0);
                    refresh();
                });
                row->addWidget(b);
            }
            layout->addLayout(row);
            contentLayout_->addWidget(maskPresets_);
            contentLayout_->addWidget(divider(content_));
        }
    }

    // Source-specific controls, replacing some of the parameter rows.
    if (!node.isShader) {
        const TextSourceState text = EngineBridge::instance()->textSourceState(nodeId_);
        if (text.valid) {
            textPanel_ = new TextSourcePanel(nodeId_, text, content_);
            contentLayout_->addWidget(textPanel_);
            contentLayout_->addWidget(divider(content_));
        }
        const FileSourceState file = EngineBridge::instance()->fileSourceState(nodeId_);
        if (file.valid) {
            filePanel_ = new FileSourcePanel(nodeId_, file, content_);
            contentLayout_->addWidget(filePanel_);
            contentLayout_->addWidget(divider(content_));
        }
        const IconSourceState icons = EngineBridge::instance()->iconSourceState(nodeId_);
        if (icons.valid) {
            iconPanel_ = new IconSourcePanel(nodeId_, icons, content_);
            contentLayout_->addWidget(iconPanel_);
            contentLayout_->addWidget(divider(content_));
        }
    }

    params_ = new QWidget(content_);
    paramsLayout_ = new QVBoxLayout(params_);
    paramsLayout_->setContentsMargins(10, 10, 10, 10);
    paramsLayout_->setSpacing(4);
    contentLayout_->addWidget(params_);

    auto* actions = new QWidget(content_);
    auto* actionsLayout = new QVBoxLayout(actions);
    actionsLayout->setContentsMargins(16, 4, 16, 4);
    actionsLayout->setSpacing(8);
    // Shader nodes and shader-generated sources carry editable GLSL.
    constexpr int kVideoSourceShader = 4;
    if (node.isShader || node.typeRaw == kVideoSourceShader) {
        auto* row = new QHBoxLayout;
        auto* b = actionButton("Edit Shader Code…", actions);
        b->setToolTip("Edit this shader's fragment GLSL for the current session");
        connect(b, &QPushButton::clicked, this, [this] { emit editShaderRequested(nodeId_, node_.name); });
        modPill_ = new QLabel("MOD", actions);
        modPill_->setToolTip("This shader has a session-only edit. Open the editor to revert.");
        modPill_->setStyleSheet(QString("color: %1; border: 1px solid %1; border-radius: 3px; padding: 1px 5px;"
                                        " font-family: Consolas; font-size: 7pt; font-weight: 700;").arg(Theme::warning.name()));
        modPill_->setVisible(EngineBridge::instance()->shaderHasOverride(nodeId_));
        row->addWidget(b, 1);
        row->addWidget(modPill_);
        actionsLayout->addLayout(row);
    }
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

    if (node.isShader && node.hasAuxOutput) {
        auto* auxBox = new QWidget(content_);
        auto* auxLayout = new QVBoxLayout(auxBox);
        auxLayout->setContentsMargins(16, 4, 16, 4);
        aux_ = new AuxStrandSection(this, node, auxBox);
        auxLayout->addWidget(aux_);
        contentLayout_->addWidget(auxBox);
    }

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
    for (const ParamInfo& p : TransformSection::applies(all) ? QVector<ParamInfo>() : all) {
        const QString y = pairedYName(p);
        if (y.isEmpty()) continue;
        for (const ParamInfo& q : all)
            if (q.name == y && q.type != ParamInfo::Hidden && (y == "Orb Y" || (q.min >= 0 && q.max <= 1.0f)))
                pairedY << q.id;
    }
    // Covered by the text panel's position pad / the icon picker.
    static const QStringList kTextPosition = {"X", "Y", "X Position", "Y Position", "Orb X", "Orb Y"};
    static const QStringList kIconPicker = {"icon", "texture", "Enable Texture"};
    // Covered by the Crop / Place pads (Scale stays: it also applies in
    // the auto-fit modes, where the Place pad is hidden).
    static const QStringList kTransformPads = {"Min X", "Max X", "Min Y", "Max Y", "X", "Y", "Center", "Scale From Center"};
    const bool transform = TransformSection::applies(all);
    QVector<ParamInfo> out;
    for (const ParamInfo& p : all) {
        if (p.type == ParamInfo::Hidden || p.hidden || kPlumbing.contains(p.name) || pairedY.contains(p.id)) continue;
        if (transform && kTransformPads.contains(p.name)) continue;
        if (textPanel_ && kTextPosition.contains(p.name)) continue;
        // The aux strand section replaces this switch (C++ spells it so).
        if (node_.hasAuxOutput && p.name == "Allow Auxillary") continue;
        if (iconPanel_ && kIconPicker.contains(p.name)) continue;
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
    transform_ = nullptr;
    const QVector<ParamInfo> visible = visibleParameters(all);
    signature_ = signature(visible);
    if (TransformSection::applies(all)) {
        transform_ = new TransformSection(node_, params_);
        transform_->setParams(all);
        paramsLayout_->addWidget(transform_);
    }

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
        addPositionPad(p, *y);
    }
    // macOS adds a Position pad for X/Y outside 0..1 too (Feedback, Swirling
    // Soul), below their sliders; not for text or transform-pipeline nodes.
    const ParamInfo* x = nullptr;
    const ParamInfo* y = nullptr;
    for (const ParamInfo& q : all) {
        if (q.type == ParamInfo::Hidden) continue;
        if (q.name == "X") x = &q;
        if (q.name == "Y") y = &q;
    }
    if (x && y && pairedYName(*x).isEmpty() && !textPanel_ && !transform_) addPositionPad(*x, *y);
    refresh();
}

// A pad over the node's output plus per-axis reactivity rows, for an X/Y
// pair mapped from each parameter's range.
void InspectorPanel::addPositionPad(const ParamInfo& x, const ParamInfo& y) {
    auto* box = new QWidget(params_);
    auto* boxLayout = new QVBoxLayout(box);
    boxLayout->setContentsMargins(6, 4, 6, 4);
    boxLayout->setSpacing(4);
    auto* label = new QLabel("Position", box);
    label->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    boxLayout->addWidget(label);
    PadGroup group;
    group.x = x;
    group.y = y;
    group.invertY = y.name != "Orb Y";  // the orb is in screen space already
    group.pad = new PositionPad(nodeId_, box);
    boxLayout->addWidget(group.pad);
    group.xRow = new ParameterRow(x, ParameterRow::Axis, "X", box);
    group.yRow = new ParameterRow(y, ParameterRow::Axis, "Y", box);
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
    if (transform_) transform_->setParams(all);
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
    if (aux_) aux_->refresh();
    if (maskPresets_)
        for (const ParamInfo& p : all)
            if (p.name == "Source Mode")
                for (QPushButton* b : maskPresets_->findChildren<QPushButton*>())
                    b->setChecked(b->property("preset").toInt() == p.intValue);
    if (modPill_) modPill_->setVisible(EngineBridge::instance()->shaderHasOverride(nodeId_));
    if (textPanel_) textPanel_->setState(EngineBridge::instance()->textSourceState(nodeId_));
    if (filePanel_) filePanel_->setState(EngineBridge::instance()->fileSourceState(nodeId_));
}

QRect InspectorPanel::parameterRowRect(const QString& paramId) const {
    const ParameterRow* row = rows_.value(paramId);
    if (!row || !row->isVisible()) return {};
    return QRect(row->mapToGlobal(QPoint(0, 0)), row->size());
}

QVector<ParamInfo> InspectorPanel::navigableParameters() const {
    QVector<QPair<int, ParamInfo>> rows;
    for (const ParameterRow* row : rows_)
        if (row->isVisibleTo(scroll_->widget()) && row->info().type != ParamInfo::Hidden)
            rows.push_back({row->mapTo(scroll_->widget(), QPoint(0, 0)).y(), row->info()});
    std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    QVector<ParamInfo> out;
    for (const auto& r : rows) out.push_back(r.second);
    return out;
}

void InspectorPanel::revealParameter(const QString& paramId) {
    if (ParameterRow* row = rows_.value(paramId)) scroll_->ensureWidgetVisible(row, 0, 24);
}

bool InspectorPanel::beginEdit(const QString& paramId, const QString& initialText) {
    ParameterRow* row = rows_.value(paramId);
    return row && row->beginEdit(initialText);
}
