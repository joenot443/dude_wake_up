#include "StageView.h"

#include "Icons.h"
#include "StageWidget.h"
#include "panels/CompactParameterControl.h"
#include "ui/ParamSelection.h"
#include "Theme.h"
#include "nodes/NodeEditorView.h"
#include "ui/Controls.h"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>
#include <deque>

namespace {
const char* const kStripMime = "application/x-nottawa-strip";
const QColor kSolo(0xff, 0xd6, 0x0a);
const QColor kBypass(0xff, 0x95, 0x00);

QFrame* hline(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    return line;
}

// The macOS StageButtonStyle: a 28 px square letter button, filled when on.
QToolButton* stageButton(const QString& letter, const QString& tip, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setText(letter);
    b->setFixedSize(28, 28);
    b->setToolTip(tip);
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

void styleStageButton(QToolButton* b, const QColor& active) {
    b->setStyleSheet(QString("QToolButton { background: %1; color: %2; border: 1px solid %3; border-radius: 6px;"
                             " font-family: Consolas; font-weight: 700; }"
                             "QToolButton:checked { background: %4; color: black; border-color: %4; }")
                         .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::border.name(), active.name()));
}
} // anon

// Centres one child at the engine's aspect ratio.
class AspectBox : public QWidget {
public:
    explicit AspectBox(QWidget* parent) : QWidget(parent) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(EngineBridge::instance(), &EngineBridge::resolutionChanged, this, [this] { place(); });
    }
    void setChild(QWidget* child) { child_ = child; place(); }
protected:
    void resizeEvent(QResizeEvent*) override { place(); }
private:
    void place() {
        if (!child_) return;
        const QSize res = EngineBridge::instance()->resolution();
        const QSize fit = (res.isEmpty() ? QSize(16, 9) : res).scaled(size(), Qt::KeepAspectRatio);
        child_->setGeometry((width() - fit.width()) / 2, (height() - fit.height()) / 2, fit.width(), fit.height());
    }
    QWidget* child_ = nullptr;
};

// ---------------------------------------------------------- channel strip

// One node's strip: drag handle, live thumbnail, name, Solo / Bypass, and
// a two-column grid of compact parameter controls.
class ChannelStrip : public QWidget {
public:
    ChannelStrip(StageView* stage, const GraphNode& node, QWidget* parent) : QWidget(parent), stage_(stage), node_(node) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(6);
        auto* header = new QHBoxLayout;
        header->setSpacing(10);
        handle_ = new QLabel(this);
        handle_->setPixmap(Icons::icon(Icons::Sliders, Theme::textTertiary).pixmap(16, 16));
        handle_->setCursor(Qt::OpenHandCursor);
        handle_->setToolTip("Drag to reorder");
        header->addWidget(handle_);
        thumb_ = new StageWidget(this, FramePresenter::NodeEditor);
        thumb_->setFixedSize(64, 64);
        thumb_->setRefreshInterval(100);
        thumb_->setSourceNode(node.id);
        header->addWidget(thumb_);
        auto* names = new QVBoxLayout;
        names->setSpacing(2);
        auto* name = new QLabel(node.name, this);
        name->setStyleSheet(QString("color: %1; font-size: 10pt; font-weight: 600;").arg(Theme::textPrimary.name()));
        auto* type = new QLabel(node.isShader ? "Effect" : "Source", this);
        type->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
        names->addWidget(name);
        names->addWidget(type);
        header->addLayout(names, 1);
        solo_ = stageButton("S", "Solo — preview this node's output", this);
        styleStageButton(solo_, kSolo);
        connect(solo_, &QToolButton::clicked, this, [this] { stage_->toggleSolo(node_.id); });
        header->addWidget(solo_);
        if (node.isShader) {
            bypass_ = stageButton("B", "Bypass — pass input through without processing", this);
            styleStageButton(bypass_, kBypass);
            bypass_->setChecked(EngineBridge::instance()->isShaderBypassed(node.id));
            connect(bypass_, &QToolButton::toggled, this, [this](bool on) {
                EngineBridge::instance()->setShaderBypassed(node_.id, on);
                update();
            });
            header->addWidget(bypass_);
        }
        layout->addLayout(header);
        grid_ = new QGridLayout;
        grid_->setColumnStretch(0, 1);
        grid_->setColumnStretch(1, 1);
        grid_->setHorizontalSpacing(8);
        grid_->setVerticalSpacing(4);
        layout->addLayout(grid_);
        empty_ = new QLabel("No favorited parameters", this);
        empty_->setStyleSheet(QString("color: %1; font-size: 8pt; padding-left: 4px;").arg(Theme::textTertiary.name()));
        layout->addWidget(empty_);
    }

    const QString& id() const { return node_.id; }

    void setSoloed(bool on) { solo_->setChecked(on); }

    // Rebuilds the controls when the shown parameters change; otherwise
    // updates their values (and Bypass, which can change elsewhere).
    void refresh(bool showAll) {
        QVector<ParamInfo> shown;
        for (const ParamInfo& p : EngineBridge::instance()->parameters(node_.id))
            if (p.type != ParamInfo::Hidden && !p.hidden && (showAll || p.favorite)) shown.push_back(p);
        QString signature = showAll ? "all" : "fav";
        for (const ParamInfo& p : shown) signature += p.id + QString::number(p.type) + ';';
        if (signature != signature_) {
            signature_ = signature;
            rebuildControls(shown);
        }
        shown_ = shown;
        for (const ParamInfo& p : shown)
            if (CompactParameterControl* c = controls_.value(p.id)) c->setInfo(p);
        empty_->setVisible(shown.isEmpty() && !showAll);
        if (bypass_) {
            const QSignalBlocker block(bypass_);
            bypass_->setChecked(EngineBridge::instance()->isShaderBypassed(node_.id));
            update();
        }
    }

    const QVector<ParamInfo>& shown() const { return shown_; }
    QWidget* control(const QString& paramId) const { return controls_.value(paramId); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath card;
        card.addRoundedRect(QRectF(rect()), 8, 8);
        const bool bypassed = bypass_ && bypass_->isChecked();
        QColor bg = Theme::surface;
        if (bypassed) bg.setAlphaF(0.5);
        p.fillPath(card, bg);
    }
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && handle_->geometry().adjusted(-6, -6, 6, 6).contains(e->position().toPoint()))
            pressPos_ = e->position().toPoint();
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (pressPos_.isNull() || (e->position().toPoint() - pressPos_).manhattanLength() < QApplication::startDragDistance()) return;
        pressPos_ = QPoint();
        auto* mime = new QMimeData;
        mime->setData(kStripMime, node_.id.toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->setPixmap(grab().scaledToWidth(240, Qt::SmoothTransformation));
        drag->exec(Qt::MoveAction);
    }
    void mouseReleaseEvent(QMouseEvent*) override { pressPos_ = QPoint(); }

private:
    void rebuildControls(const QVector<ParamInfo>& params) {
        while (QLayoutItem* item = grid_->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        controls_.clear();
        int i = 0;
        for (const ParamInfo& p : params) {
            auto* control = new CompactParameterControl(p, this);
            // An unfavourited parameter leaves the favourites-only list.
            connect(control, &CompactParameterControl::favoritesChanged, stage_, [this] {
                QMetaObject::invokeMethod(stage_, [stage = stage_] { stage->refreshNow(); }, Qt::QueuedConnection);
            });
            controls_.insert(p.id, control);
            grid_->addWidget(control, i / 2, i % 2, Qt::AlignTop);
            ++i;
        }
    }

    StageView*   stage_;
    GraphNode    node_;
    QLabel*      handle_;
    StageWidget* thumb_;
    QToolButton* solo_;
    QToolButton* bypass_ = nullptr;
    QGridLayout* grid_;
    QLabel*      empty_;
    QHash<QString, CompactParameterControl*> controls_;
    QVector<ParamInfo> shown_;
    QString      signature_;
    QPoint       pressPos_;
};

// ------------------------------------------------------------- stage view

StageView::StageView(NodeEditorView* editor, QWidget* parent) : QWidget(parent), editor_(editor) {
    setObjectName("StageView");
    setStyleSheet(QString("#StageView { background: %1; }").arg(Theme::background.name()));
    setAcceptDrops(true);
    QSettings settings;
    order_ = settings.value("stage/order").toStringList();
    showAll_ = settings.value("stage/showAll", true).toBool();
    auto* eng = EngineBridge::instance();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Empty state.
    emptyState_ = new QWidget(this);
    auto* emptyLayout = new QVBoxLayout(emptyState_);
    emptyLayout->addStretch(1);
    auto* emptyIcon = new QLabel(emptyState_);
    emptyIcon->setPixmap(Icons::icon(Icons::Sliders, Theme::textTertiary).pixmap(40, 40));
    emptyIcon->setAlignment(Qt::AlignCenter);
    auto* emptyText = new QLabel("No active chains yet. Set up your visuals in the node editor, then come back here to perform.", emptyState_);
    emptyText->setWordWrap(true);
    emptyText->setAlignment(Qt::AlignCenter);
    emptyText->setStyleSheet(QString("color: %1;").arg(Theme::textSecondary.name()));
    auto* exit = new QPushButton("Exit Stage Mode", emptyState_);
    connect(exit, &QPushButton::clicked, this, &StageView::exitRequested);
    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addWidget(exit, 0, Qt::AlignCenter);
    emptyLayout->addStretch(1);
    root->addWidget(emptyState_);

    content_ = new QWidget(this);
    auto* columns = new QHBoxLayout(content_);
    columns->setContentsMargins(0, 0, 0, 0);
    columns->setSpacing(0);
    root->addWidget(content_, 1);

    // Left: strips.
    auto* left = new QWidget(content_);
    left->setMinimumWidth(350);
    left->setMaximumWidth(520);
    left->setObjectName("StageStrips");
    left->setStyleSheet(QString("#StageStrips { background: %1; }").arg(Theme::backgroundSecondary.name()));
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    auto* filter = new QHBoxLayout;
    filter->setContentsMargins(12, 8, 12, 8);
    favoritesButton_ = new QToolButton(left);
    favoritesButton_->setIconSize(QSize(13, 13));
    favoritesButton_->setCursor(Qt::PointingHandCursor);
    favoritesLabel_ = new QLabel(left);
    favoritesLabel_->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    connect(favoritesButton_, &QToolButton::clicked, this, [this] {
        showAll_ = !showAll_;
        QSettings().setValue("stage/showAll", showAll_);
        refreshValues();
    });
    resetOrder_ = new QToolButton(left);
    resetOrder_->setIcon(Icons::icon(Icons::Reset, Theme::textSecondary));
    resetOrder_->setIconSize(QSize(11, 11));
    resetOrder_->setToolTip("Reset to default order");
    connect(resetOrder_, &QToolButton::clicked, this, [this] {
        order_.clear();
        QSettings().remove("stage/order");
        signature_.clear();
        rebuild();
    });
    filter->addWidget(favoritesButton_);
    filter->addWidget(favoritesLabel_);
    filter->addStretch(1);
    filter->addWidget(resetOrder_);
    leftLayout->addLayout(filter);
    leftLayout->addWidget(hline(left));
    auto* scroll = new QScrollArea(left);
    scroll_ = scroll;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    stripList_ = new QWidget(scroll);
    stripList_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);  // fit the column
    stripLayout_ = new QVBoxLayout(stripList_);
    stripLayout_->setContentsMargins(12, 8, 12, 8);
    stripLayout_->setSpacing(8);
    scroll->setWidget(stripList_);
    leftLayout->addWidget(scroll, 1);
    dropIndicator_ = new QWidget(stripList_);
    dropIndicator_->setFixedHeight(3);
    dropIndicator_->setStyleSheet(QString("background: %1; border-radius: 1px;").arg(Theme::accent.name()));
    dropIndicator_->hide();
    columns->addWidget(left);
    auto* divider = new QFrame(content_);
    divider->setFixedWidth(1);
    divider->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    columns->addWidget(divider);

    // Right: output.
    auto* right = new QWidget(content_);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    auto* header = new QWidget(right);
    header->setObjectName("StageHeader");
    header->setStyleSheet(QString("#StageHeader { background: %1; }").arg(Theme::backgroundSecondary.name()));
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 12, 8);
    headerLayout->setSpacing(8);
    const QString capsule = QString("QToolButton { background: %1; color: %2; border: none; border-radius: 11px;"
                                    " padding: 3px 10px; font-size: 8pt; font-weight: 600; }"
                                    "QToolButton:hover { background: %3; }")
                                .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::surfaceHover.name());
    outputPicker_ = new QToolButton(header);
    outputPicker_->setStyleSheet(capsule);
    outputPicker_->setCursor(Qt::PointingHandCursor);
    outputPicker_->setToolTip("Choose what the monitor shows");
    connect(outputPicker_, &QToolButton::clicked, this, [this] {
        QMenu menu(this);
        QAction* autoAction = menu.addAction("Auto (deepest output)", this, [this] { pinnedId_.clear(); updateMonitor(); });
        autoAction->setCheckable(true);
        autoAction->setChecked(pinnedId_.isEmpty());
        menu.addSeparator();
        for (const Strip& s : std::as_const(strips_)) {
            QAction* a = menu.addAction(s.node.name, this, [this, id = s.node.id] { pinnedId_ = id; updateMonitor(); });
            a->setCheckable(true);
            a->setChecked(pinnedId_ == s.node.id);
        }
        menu.exec(outputPicker_->mapToGlobal(QPoint(0, outputPicker_->height())));
    });
    soloBadge_ = new QToolButton(header);
    soloBadge_->setStyleSheet(QString("QToolButton { background: %1; color: black; border: none; border-radius: 10px;"
                                      " padding: 3px 8px; font-size: 7pt; font-weight: 700; }").arg(kSolo.name()));
    soloBadge_->setCursor(Qt::PointingHandCursor);
    soloBadge_->setToolTip("Click to clear solo");
    connect(soloBadge_, &QToolButton::clicked, this, [this] { toggleSolo(soloId_); });
    auto* popOut = new QToolButton(header);
    popOut->setText("Pop Out");
    popOut->setToolTip("Open in a separate window");
    popOut->setCursor(Qt::PointingHandCursor);
    popOut->setStyleSheet(QString("QToolButton { background: %1; color: %2; border: none; border-radius: 6px;"
                                  " padding: 3px 8px; font-size: 8pt; }"
                                  "QToolButton:hover { background: %3; }")
                              .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::surfaceHover.name()));
    connect(popOut, &QToolButton::clicked, this, [this] {
        const QString id = outputId();
        if (!id.isEmpty()) editor_->openPreview(id);
    });
    headerLayout->addWidget(outputPicker_);
    headerLayout->addWidget(soloBadge_);
    headerLayout->addStretch(1);
    headerLayout->addWidget(popOut);
    rightLayout->addWidget(header);
    rightLayout->addWidget(hline(right));

    auto* monitorBox = new QWidget(right);
    auto* monitorLayout = new QVBoxLayout(monitorBox);
    monitorLayout->setContentsMargins(12, 12, 12, 12);
    monitorLayout->setSpacing(12);
    // The monitor keeps the output's aspect ratio, centred in the space.
    auto* monitorArea = new AspectBox(monitorBox);
    monitor_ = new StageWidget(monitorArea, FramePresenter::Stage);
    monitor_->setMinimumSize(160, 90);
    monitorArea->setChild(monitor_);
    monitorLayout->addWidget(monitorArea, 1);
    monitorLabel_ = new QLabel(monitor_);
    monitorLabel_->setStyleSheet("QLabel { background: rgba(0,0,0,150); color: white; border-radius: 4px; padding: 3px 6px;"
                                 " font-family: Consolas; font-size: 8pt; font-weight: 700; }");
    monitorLabel_->move(8, 8);
    windowsBox_ = new QWidget(monitorBox);
    windowsLayout_ = new QVBoxLayout(windowsBox_);
    windowsLayout_->setContentsMargins(0, 0, 0, 0);
    windowsLayout_->setSpacing(2);
    monitorLayout->addWidget(windowsBox_);
    rightLayout->addWidget(monitorBox, 1);
    rightLayout->addWidget(hline(right));

    auto* bottom = new QWidget(right);
    bottom->setObjectName("StageBottom");
    bottom->setStyleSheet(QString("#StageBottom { background: %1; }").arg(Theme::backgroundSecondary.name()));
    auto* bottomLayout = new QHBoxLayout(bottom);
    bottomLayout->setContentsMargins(16, 8, 16, 8);
    bpm_ = new QLabel(bottom);
    bpm_->setStyleSheet(QString("color: %1; font-family: Consolas; font-size: 10pt;").arg(Theme::textSecondary.name()));
    undo_ = new QToolButton(bottom);
    redo_ = new QToolButton(bottom);
    undo_->setToolTip("Undo (Ctrl+Z)");
    redo_->setToolTip("Redo (Ctrl+Shift+Z)");
    connect(undo_, &QToolButton::clicked, this, [eng] { eng->undo(); });
    connect(redo_, &QToolButton::clicked, this, [eng] { eng->redo(); });
    bottomLayout->addWidget(bpm_);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(undo_);
    bottomLayout->addWidget(redo_);
    rightLayout->addWidget(bottom);
    columns->addWidget(right, 1);

    refresh_.setInterval(250);
    connect(&refresh_, &QTimer::timeout, this, [this] { refreshValues(); updateMonitor(); });
}

void StageView::showEvent(QShowEvent*) {
    signature_.clear();
    rebuild();
    refresh_.start();
}

void StageView::hideEvent(QHideEvent*) {
    refresh_.stop();
    monitor_->setSourceNode({});
    soloId_.clear();
    pinnedId_.clear();
    // Strips hold shared textures; drop them while the stage is hidden.
    for (ChannelStrip* s : std::as_const(stripWidgets_)) s->deleteLater();
    stripWidgets_.clear();
    signature_.clear();
}

// Depth from the roots (BFS, deepest wins) and chain per root, as macOS does.
QVector<StageView::Strip> StageView::computeStrips() const {
    auto* eng = EngineBridge::instance();
    const QVector<GraphNode> nodes = eng->nodes();
    const QVector<GraphEdge> edges = eng->edges();
    QHash<QString, QStringList> children;
    QHash<QString, int> inDegree;
    for (const GraphNode& n : nodes) inDegree[n.id] = 0;
    for (const GraphEdge& e : edges) { children[e.startId] << e.endId; inDegree[e.endId] += 1; }
    QHash<QString, int> depth, chain;
    int currentChain = 0;
    for (const GraphNode& root : nodes) {
        if (inDegree.value(root.id) != 0 || depth.contains(root.id)) continue;
        depth[root.id] = 0;
        chain[root.id] = currentChain;
        std::deque<QString> queue{root.id};
        while (!queue.empty()) {
            const QString cur = queue.front();
            queue.pop_front();
            for (const QString& child : children.value(cur)) {
                const int d = depth.value(cur) + 1;
                if (!depth.contains(child) || d > depth.value(child)) {
                    depth[child] = d;
                    chain[child] = chain.value(cur);
                    queue.push_back(child);
                }
            }
        }
        ++currentChain;
    }
    QVector<Strip> strips;
    for (const GraphNode& n : nodes)
        strips.push_back({n, chain.value(n.id), depth.value(n.id), children.value(n.id).isEmpty()});
    std::stable_sort(strips.begin(), strips.end(), [](const Strip& a, const Strip& b) {
        return a.chain != b.chain ? a.chain < b.chain : a.depth < b.depth;
    });
    if (!order_.isEmpty()) {
        QVector<Strip> ordered;
        for (const QString& id : order_)
            for (const Strip& s : strips) if (s.node.id == id) ordered.push_back(s);
        for (const Strip& s : strips) if (!order_.contains(s.node.id)) ordered.push_back(s);
        strips = ordered;
    }
    return strips;
}

void StageView::rebuild() {
    if (!isVisible()) return;
    strips_ = computeStrips();
    QString signature;
    for (const Strip& s : std::as_const(strips_)) signature += s.node.id + s.node.name + ';';
    const bool empty = strips_.isEmpty();
    emptyState_->setVisible(empty);
    content_->setVisible(!empty);
    resetOrder_->setVisible(!order_.isEmpty());
    if (signature != signature_) {
        signature_ = signature;
        while (QLayoutItem* item = stripLayout_->takeAt(0)) delete item;
        QHash<QString, ChannelStrip*> kept;
        for (const Strip& s : std::as_const(strips_)) {
            ChannelStrip* w = stripWidgets_.take(s.node.id);
            if (!w) w = new ChannelStrip(this, s.node, stripList_);
            kept.insert(s.node.id, w);
            stripLayout_->addWidget(w);
        }
        for (ChannelStrip* gone : std::as_const(stripWidgets_)) gone->deleteLater();
        stripWidgets_ = kept;
        stripLayout_->addStretch(1);
    }
    refreshValues();
    updateMonitor();
}

void StageView::refreshValues() {
    favoritesButton_->setIcon(Icons::icon(showAll_ ? Icons::Star : Icons::StarFilled, showAll_ ? Theme::textSecondary : Theme::accent));
    favoritesButton_->setToolTip(showAll_ ? "Show favorites only" : "Showing favorites only");
    favoritesLabel_->setText(showAll_ ? "All Parameters" : "Favorites Only");
    for (ChannelStrip* s : std::as_const(stripWidgets_)) {
        s->refresh(showAll_);
        s->setSoloed(s->id() == soloId_);
    }
    auto* eng = EngineBridge::instance();
    bpm_->setText(QString("%1 BPM").arg(int(eng->currentBpm())));
    const bool canUndo = eng->canUndo(), canRedo = eng->canRedo();
    undo_->setEnabled(canUndo);
    redo_->setEnabled(canRedo);
    undo_->setIcon(Icons::icon(Icons::Undo, canUndo ? Theme::textSecondary : Theme::textTertiary.darker(130)));
    redo_->setIcon(Icons::icon(Icons::Redo, canRedo ? Theme::textSecondary : Theme::textTertiary.darker(130)));

    // Open preview windows, with close buttons.
    const QStringList open = editor_->openPreviewIds();
    if (open.join(';') != windowsSignature_) {
        windowsSignature_ = open.join(';');
        while (QLayoutItem* item = windowsLayout_->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        if (!open.isEmpty()) {
            auto* heading = new QLabel("WINDOWS", windowsBox_);
            heading->setStyleSheet(QString("color: %1; font-family: Consolas; font-size: 7pt; font-weight: 700;").arg(Theme::textTertiary.name()));
            windowsLayout_->addWidget(heading);
        }
        for (const QString& id : open) {
            QString name = id;
            for (const Strip& s : std::as_const(strips_)) if (s.node.id == id) name = s.node.name;
            auto* row = new QWidget(windowsBox_);
            row->setObjectName("WindowRow");
            row->setStyleSheet(QString("#WindowRow { background: %1; border-radius: 5px; }").arg(Theme::surface.name()));
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(8, 4, 8, 4);
            auto* label = new QLabel(name, row);
            label->setStyleSheet(QString("color: %1; font-size: 8pt; background: transparent;").arg(Theme::textPrimary.name()));
            auto* close = new QToolButton(row);
            close->setIcon(Icons::icon(Icons::Close, Theme::textSecondary));
            close->setIconSize(QSize(10, 10));
            close->setStyleSheet("background: transparent; border: none;");
            connect(close, &QToolButton::clicked, this, [this, id] { editor_->closePreview(id); });
            rowLayout->addWidget(label, 1);
            rowLayout->addWidget(close);
            windowsLayout_->addWidget(row);
        }
    }
    windowsBox_->setVisible(!open.isEmpty());
}

// Solo > pinned > the deepest node with no outgoing connections.
QString StageView::outputId() const {
    auto has = [this](const QString& id) {
        return std::any_of(strips_.begin(), strips_.end(), [&](const Strip& s) { return s.node.id == id; });
    };
    if (!soloId_.isEmpty() && has(soloId_)) return soloId_;
    if (!pinnedId_.isEmpty() && has(pinnedId_)) return pinnedId_;
    const Strip* best = nullptr;
    for (const Strip& s : strips_) if (s.terminal && (!best || s.depth > best->depth)) best = &s;
    if (best) return best->node.id;
    return strips_.isEmpty() ? QString() : strips_.last().node.id;
}

void StageView::updateMonitor() {
    const QString id = outputId();
    monitor_->setSourceNode(id);
    QString name = "No Output";
    QString pinnedName;
    for (const Strip& s : std::as_const(strips_)) {
        if (s.node.id == id) name = s.node.name;
        if (s.node.id == pinnedId_) pinnedName = s.node.name;
    }
    monitorLabel_->setText(name);
    monitorLabel_->adjustSize();
    monitorLabel_->setVisible(!id.isEmpty());
    outputPicker_->setText("▣  " + (pinnedName.isEmpty() ? QStringLiteral("Auto") : pinnedName) + "  ⌄");
    QString soloName;
    for (const Strip& s : std::as_const(strips_)) if (s.node.id == soloId_) soloName = s.node.name;
    soloBadge_->setText("●  SOLO: " + soloName);
    soloBadge_->setVisible(!soloName.isEmpty());
}

void StageView::toggleSolo(const QString& id) {
    soloId_ = soloId_ == id ? QString() : id;
    for (ChannelStrip* s : std::as_const(stripWidgets_)) s->setSoloed(s->id() == soloId_);
    updateMonitor();
}

// ------------------------------------------------------------- reordering

int StageView::dropIndexAt(QPoint pos) const {
    const QPoint inList = stripList_->mapFrom(this, pos);
    int index = 0;
    for (const Strip& s : strips_) {
        ChannelStrip* w = stripWidgets_.value(s.node.id);
        if (w && inList.y() > w->geometry().center().y()) ++index;
    }
    return index;
}

void StageView::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasFormat(kStripMime)) e->acceptProposedAction();
}

void StageView::dragMoveEvent(QDragMoveEvent* e) {
    if (!e->mimeData()->hasFormat(kStripMime)) return;
    e->acceptProposedAction();
    const int index = dropIndexAt(e->position().toPoint());
    int y = 4;
    if (index < strips_.size()) {
        if (ChannelStrip* w = stripWidgets_.value(strips_[index].node.id)) y = w->geometry().top() - 6;
    } else if (!strips_.isEmpty()) {
        if (ChannelStrip* w = stripWidgets_.value(strips_.last().node.id)) y = w->geometry().bottom() + 3;
    }
    dropIndicator_->setGeometry(12, y, stripList_->width() - 24, 3);
    dropIndicator_->show();
    dropIndicator_->raise();
}

void StageView::dragLeaveEvent(QDragLeaveEvent*) { dropIndicator_->hide(); }

void StageView::dropEvent(QDropEvent* e) {
    dropIndicator_->hide();
    const QString id = QString::fromUtf8(e->mimeData()->data(kStripMime));
    int target = dropIndexAt(e->position().toPoint());
    QStringList ids;
    for (const Strip& s : std::as_const(strips_)) ids << s.node.id;
    const int from = int(ids.indexOf(id));
    if (from < 0) return;
    e->acceptProposedAction();
    ids.removeAt(from);
    if (target > from) --target;
    ids.insert(std::clamp(target, 0, int(ids.size())), id);
    order_ = ids;
    QSettings().setValue("stage/order", order_);
    signature_.clear();
    rebuild();
}

void StageView::refreshNow() { refreshValues(); }

QVector<ParamInfo> StageView::navigableParameters() const {
    QVector<ParamInfo> out;
    for (const Strip& s : strips_)
        if (ChannelStrip* w = stripWidgets_.value(s.node.id)) out += w->shown();
    return out;
}

void StageView::revealParameter(const QString& paramId) {
    for (ChannelStrip* w : std::as_const(stripWidgets_))
        if (QWidget* c = w->control(paramId)) scroll_->ensureWidgetVisible(c, 0, 24);
}
