#include "HelpGuide.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "nodes/NodeEditorView.h"
#include "panels/BrowserPanel.h"
#include "panels/InspectorPanel.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>
#include <cmath>
#include <functional>

namespace HelpStrings {
const char* welcomeTitle = "Welcome to Nottawa!";
const char* welcomeSubtitle = "Thank you so much for being here.\n\nTo skip this tutorial, click the ? icon in the bottom right.";
const char* startGuide = "Everything in Nottawa starts with a Source. A Source can be a live generated scene (\"effect\"), "
                         "video file, webcam feed, and more.";
const char* sourceDragInstruction = "To start, click and drag a Source from the sidebar over to the canvas.";
const char* addEffectTitle = "Add an Effect";
const char* effectDragInstruction = "Grab any Effect you see in the sidebar and drag it to the canvas. If you're not sure, "
                                    "try Kaleidoscope or Pixelate.";
const char* connectSourceEffectTitle = "Let's connect our Source to the Effect.";
const char* connectInstruction = "Click and drag from the filled pin on the right side of the Source\nto the hollow pin on "
                                 "the left side of the Effect.";
const char* completeConnectionTitle = "Complete the connection";
const char* dragToEffectInstruction = "Drag to the hollow pin on your Effect.";
const char* addSecondSourceTitle = "Adding another Source";
const char* addSecondSourceInstruction = "Add a second Source to the canvas.";
const char* webcamVideoInstruction = "You can also use a Webcam or video from the Library.";
const char* openShaderInfoTitle = "Open the Effect Settings";
const char* clickBlendInstruction = "Click inside the Blend node or its preview to open its Settings pane.";
const char* changeParametersTitle = "Blend Settings";
const char* blendModeInstruction = "First, select a different Mode to change how the images blend.";
const char* alphaModeInstruction = "Now, use the Main Input Strength slider to change the opacity of the blend.";
const char* otherNodesInstruction = "You can click on other nodes to change their settings, too.";
const char* blendViaOutputsTitle = "Create a Blend";
const char* blendViaOutputsInstruction = "You can blend two nodes together by connecting their output pins.\n\nDrag from "
                                         "this output pin to the output pin on the other node.";
const char* blendViaOutputsDrop = "Drop it on this output pin to create a Blend.";
const char* actionButtonsTitle = "Common Actions";
const char* undoInstruction = "Undoes the last action";
const char* redoInstruction = "Redoes the last action";
const char* cameraViewInstruction = "Saves a screenshot of the selected node or the output";
const char* helpToggleInstruction = "Turns this tutorial on/off";
const char* clearCanvasInstruction = "Clears the canvas, deleting all nodes and connections";
const char* scaleToFitInstruction = "Scales the canvas to fit the content";
const char* shareStrandTitle = "Share Your Creation";
const char* shareStrandDescription = "You can share your node setup with others!";
const char* shareStrandStep1 = "Right-click any node and select \"Share Strand…\"";
const char* shareStrandStep2 = "Add a title and description for your creation";
const char* shareStrandStep3 = "The preview image is captured automatically from your output";
const char* shareStrandStep4 = "Click Share to upload and get a link you can share anywhere!";
const char* shareStrandNote = "Others can open your strand directly in Nottawa from the link.";
const char* completionTitle = "Thank you for being a part of Nottawa!";
const char* completionDescription = "I really appreciate you taking the time to try out this app. The goal of Nottawa is to "
                                    "make live video available for everyone, the best way for me to do that is to listen to you!";
const char* completionDescription2 = "I hope you can continue playing around and make something cool. Try using a Webcam, "
                                     "videos from the Library, or ones you've filmed yourself.";
const char* completionDescription3 = "Once you're done, I'd love if you could share your thoughts. Use the menu option above, "
                                     "join the Discord, post in /r/nottawa_app, or just send me a message.\n\nWhat was cool? "
                                     "What was confusing? What was broken? What was missing? Everything helps!";
const char* completionDescription4 = "I love you all -\nJoe";
} // namespace HelpStrings

namespace {
constexpr int kBlendType = 80;
const char* const kCompletedKey = "help/completed";
const char* const kAutostartedKey = "help/autostarted";

QLabel* text(const QString& s, const QColor& color, int pt, QWidget* parent, bool bold = false) {
    auto* l = new QLabel(s, parent);
    l->setWordWrap(true);
    l->setStyleSheet(QString("color: %1; font-size: %2pt; %3").arg(color.name()).arg(pt).arg(bold ? "font-weight: 600;" : ""));
    return l;
}
} // anon

// The accent arrow beside a card, bobbing 5 px towards what it points at.
class HelpArrow : public QWidget {
public:
    enum Direction { Left, Right, Up, Down };
    HelpArrow(Direction d, QWidget* parent) : QWidget(parent), dir_(d) {
        setFixedSize(d == Left || d == Right ? QSize(40, 32) : QSize(32, 40));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setObjectName("HelpArrow");
        auto* t = new QTimer(this);
        connect(t, &QTimer::timeout, this, qOverload<>(&QWidget::update));
        t->start(33);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const double off = std::sin(QDateTime::currentMSecsSinceEpoch() / 1000.0 * 5.0) * 5.0;
        p.translate(width() / 2.0, height() / 2.0);
        p.rotate(dir_ == Left ? 180 : dir_ == Up ? -90 : dir_ == Down ? 90 : 0);
        p.translate(off, 0);
        p.setPen(QPen(Theme::accent, 3.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(QPointF(-12, 0), QPointF(12, 0));
        p.drawPolyline(QPolygonF({QPointF(3, -9), QPointF(12, 0), QPointF(3, 9)}));
    }
private:
    Direction dir_;
};

// A tutorial card: title, body and optional extras, with an arrow on one
// side. Cards without buttons let clicks through.
class HelpCard : public QWidget {
public:
    HelpCard(QWidget* parent, int maxWidth = 320) : QWidget(parent) {
        outer_ = new QBoxLayout(QBoxLayout::LeftToRight, this);
        outer_->setContentsMargins(0, 0, 0, 0);
        outer_->setSpacing(8);
        body_ = new QWidget(this);
        body_->setMaximumWidth(maxWidth);
        column_ = new QVBoxLayout(body_);
        column_->setContentsMargins(16, 16, 16, 16);
        column_->setSpacing(8);
        outer_->addWidget(body_);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        // Only the painted card shows, not the app stylesheet's panel fill.
        setObjectName("HelpCard");
        body_->setObjectName("HelpBody");
        setStyleSheet("#HelpCard, #HelpBody, #HelpArrow { background: transparent; }");
    }
    HelpCard* title(const QString& s, int pt = 11) { column_->addWidget(text(s, Theme::textPrimary, pt, body_, true)); return this; }
    HelpCard* body(const QString& s, const QColor& c = Theme::textSecondary, int pt = 9) {
        column_->addWidget(text(s, c, pt, body_));
        return this;
    }
    HelpCard* iconRow(Icons::Kind icon, const QString& s) {
        auto* row = new QHBoxLayout;
        row->setSpacing(8);
        auto* i = new QLabel(body_);
        i->setPixmap(Icons::icon(icon, Theme::textSecondary).pixmap(16, 16));
        i->setFixedWidth(20);
        row->addWidget(i);
        row->addWidget(text(s, Theme::textSecondary, 9, body_), 1);
        column_->addLayout(row);
        return this;
    }
    HelpCard* bullet(const QString& s) { return body("•  " + s, Theme::textSecondary, 8); }
    HelpCard* divider() {
        auto* line = new QFrame(body_);
        line->setFixedHeight(1);
        QColor c = Theme::border;
        c.setAlphaF(0.3);
        line->setStyleSheet(QString("background: %1;").arg(c.name(QColor::HexArgb)));
        column_->addWidget(line);
        return this;
    }
    HelpCard* button(const QString& label, std::function<void()> onClick, bool centred = false, bool large = false) {
        auto* b = new QPushButton(label, body_);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString("QPushButton { background: %1; color: black; border: none; border-radius: 6px;"
                                 " padding: %3; font-weight: 600; }"
                                 "QPushButton:hover { background: %2; }")
                             .arg(Theme::accent.name(), Theme::accentHover.name(), large ? "8px 22px" : "5px 14px"));
        QObject::connect(b, &QPushButton::clicked, body_, std::move(onClick));
        column_->addWidget(b, 0, centred ? Qt::AlignHCenter : Qt::AlignLeft);
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
        return this;
    }
    // Places an arrow on the given side, pointing away from the card.
    HelpCard* arrow(HelpArrow::Direction d) {
        auto* a = new HelpArrow(d, this);
        switch (d) {
        case HelpArrow::Left:  outer_->setDirection(QBoxLayout::LeftToRight); outer_->insertWidget(0, a, 0, Qt::AlignVCenter); break;
        case HelpArrow::Right: outer_->setDirection(QBoxLayout::LeftToRight); outer_->addWidget(a, 0, Qt::AlignVCenter); break;
        case HelpArrow::Up:    outer_->setDirection(QBoxLayout::TopToBottom); outer_->insertWidget(0, a, 0, Qt::AlignHCenter); break;
        case HelpArrow::Down:  outer_->setDirection(QBoxLayout::TopToBottom); outer_->addWidget(a, 0, Qt::AlignHCenter); break;
        }
        return this;
    }
    void finish() {
        adjustSize();
        show();
        raise();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(body_->geometry()).adjusted(0.5, 0.5, -0.5, -0.5);
        for (int i = 6; i > 0; i -= 2) {  // soft shadow
            QPainterPath s;
            s.addRoundedRect(r.adjusted(-i, -i + 4, i, i + 4), 10 + i, 10 + i);
            p.fillPath(s, QColor(0, 0, 0, 14));
        }
        QPainterPath card;
        card.addRoundedRect(r, 10, 10);
        QColor bg = Theme::surface;
        bg.setAlphaF(0.95);
        p.fillPath(card, bg);
        QColor border = Theme::accent;
        border.setAlphaF(0.3);
        p.setPen(QPen(border, 1));
        p.drawPath(card);
    }

private:
    QBoxLayout*  outer_;
    QWidget*     body_;
    QVBoxLayout* column_;
};

HelpGuide::HelpGuide(QWidget* canvas, NodeEditorView* editor, InspectorPanel* inspector, BrowserPanel* browser, QObject* parent)
    : QObject(parent), canvas_(canvas), editor_(editor), inspector_(inspector), browser_(browser) {
    completed_ = QSettings().value(kCompletedKey, false).toBool();
    timer_.setInterval(33);  // node tooltips follow their nodes
    connect(&timer_, &QTimer::timeout, this, &HelpGuide::tick);
}

void HelpGuide::setEnabled(bool on) {
    if (on == enabled_) return;
    enabled_ = on;
    lastStep_ = -1;
    ticks_ = 0;
    if (active()) timer_.start();
    else { timer_.stop(); hideAll(); }
    emit enabledChanged(on);
}

void HelpGuide::restart() {
    editedMode_ = editedAlpha_ = seenActions_ = seenShare_ = false;
    blendId_.clear();
    setCompleted(false);
    enabled_ = false;
    setEnabled(true);
}

void HelpGuide::autoStartIfFirstLaunch() {
    QSettings settings;
    if (settings.contains(kAutostartedKey)) return;
    settings.setValue(kAutostartedKey, true);
    if (!completed_) setEnabled(true);
}

void HelpGuide::setCompleted(bool on) {
    completed_ = on;
    QSettings().setValue(kCompletedKey, on);
    if (on) { timer_.stop(); hideAll(); }
}

bool HelpGuide::active() const { return enabled_ && !completed_; }

HelpGuide::Step HelpGuide::currentStep() {
    auto* eng = EngineBridge::instance();
    const QVector<GraphNode> nodes = eng->nodes();
    int sources = 0, shaders = 0;
    QString blend;
    for (const GraphNode& n : nodes) {
        if (n.isShader) ++shaders; else ++sources;
        if (n.isShader && n.typeRaw == kBlendType && blend.isEmpty()) blend = n.id;
    }
    if (sources == 0) return PlaceSource;
    if (shaders == 0) return PlaceEffect;
    if (eng->edges().isEmpty()) return ConnectSourceToEffect;
    if (sources < 2) return PlaceSecondSource;
    if (blend.isEmpty()) return BlendViaOutputs;

    // Notice edits to the Blend's Mode and Main Input Strength.
    if (blend != blendId_) { blendId_ = blend; modeAtStart_ = -1; alphaAtStart_ = -1; }
    for (const ParamInfo& p : eng->parameters(blend)) {
        if (p.name == "Mode") {
            if (modeAtStart_ < 0) modeAtStart_ = p.intValue;
            else if (p.intValue != modeAtStart_) editedMode_ = true;
        } else if (p.name == "Main Input Strength") {
            if (alphaAtStart_ < 0) alphaAtStart_ = p.value;
            else if (std::abs(p.value - alphaAtStart_) > 0.001f && editedMode_) editedAlpha_ = true;
        }
    }
    if (!editor_->selectedNodeIds().contains(blend)) {
        if (!editedMode_ || !editedAlpha_) return OpenInspector;
    } else {
        if (!editedMode_) return EditBlendMode;
        if (!editedAlpha_) return EditAlpha;
    }
    if (!seenActions_) return ActionButtonsTour;
    if (!seenShare_) return ShareHelp;
    return Completion;
}

void HelpGuide::hideAll() {
    if (card_) card_->deleteLater();
    if (nodeCard_) nodeCard_->deleteLater();
    tipTarget_.clear();
}

void HelpGuide::tick() {
    if (!active()) return;
    // Out of the way of the welcome screen and stage mode.
    const bool covered = (blocker_ && blocker_->isVisible()) || !editor_->isVisible();
    if (covered) {
        if (card_) card_->hide();
        if (nodeCard_) nodeCard_->hide();
        return;
    }
    if (card_ && card_->isHidden()) { card_->show(); card_->raise(); }
    const QSize area = canvas_->size();
    // Work out the step a few times a second; follow nodes every frame.
    if (ticks_++ % 6 == 0 || area != lastArea_) {
        const Step step = currentStep();
        if (step != lastStep_ || area != lastArea_) {
            const bool stepChanged = step != lastStep_;
            lastStep_ = step;
            lastArea_ = area;
            if (card_) card_->deleteLater();
            showCards(step);
            if (stepChanged && (step == PlaceSource || step == PlaceSecondSource)) {
                browser_->showTab(BrowserPanel::Sources);
                emit sidebarNeeded();
            }
            if (stepChanged && step == PlaceEffect) {
                browser_->showTab(BrowserPanel::Effects);
                emit sidebarNeeded();
            }
        }
        updateTooltipTarget(step);
        if (card_ && (step == EditBlendMode || step == EditAlpha)) {
            QString paramId;
            for (const ParamInfo& p : EngineBridge::instance()->parameters(blendId_))
                if (p.name == (step == EditBlendMode ? "Mode" : "Main Input Strength")) paramId = p.id;
            const QRect row = inspector_->parameterRowRect(paramId);
            int y = row.isNull() ? area.height() / 3 : canvas_->mapFromGlobal(row.center()).y();
            y = std::clamp(y, card_->height() / 2 + 8, std::max(card_->height() / 2 + 8, area.height() - card_->height() / 2 - 8));
            card_->move(area.width() - card_->width() - 8, y - card_->height() / 2);
        }
    }

    // The tooltip beside a node (connect, blend, select the Blend).
    if (tipTarget_.isEmpty()) {
        if (nodeCard_) nodeCard_->deleteLater();
        return;
    }
    const QRect node = editor_->nodeViewportRect(tipTarget_);
    if (node.isNull()) { if (nodeCard_) nodeCard_->hide(); return; }
    const QRect r(editor_->viewport()->mapTo(canvas_, node.topLeft()), node.size());
    // Beside the node on the preferred side, or the other side if it
    // doesn't fit there.
    auto dir = HelpArrow::Direction(tipDir_);
    const int cardWidth = nodeCard_ ? nodeCard_->width() : 370;
    if (dir == HelpArrow::Left && r.right() + 8 + cardWidth > area.width() && r.left() - 8 - cardWidth >= 0)
        dir = HelpArrow::Right;
    else if (dir == HelpArrow::Right && r.left() - 8 - cardWidth < 0 && r.right() + 8 + cardWidth <= area.width())
        dir = HelpArrow::Left;
    const QString key = tipTarget_ + tipTitle_ + QString::number(dir);
    if (!nodeCard_ || nodeCard_->property("key").toString() != key) {
        if (nodeCard_) nodeCard_->deleteLater();
        nodeCard_ = new HelpCard(canvas_);
        nodeCard_->setProperty("key", key);
        nodeCard_->title(tipTitle_);
        if (!tipBody_.isEmpty()) nodeCard_->body(tipBody_);
        nodeCard_->arrow(dir)->finish();
    }
    QPoint at;
    if (dir == HelpArrow::Left) at = QPoint(r.right() + 8, r.center().y() - nodeCard_->height() / 2);
    else if (dir == HelpArrow::Right) at = QPoint(r.left() - 8 - nodeCard_->width(), r.center().y() - nodeCard_->height() / 2);
    else at = QPoint(r.center().x() - nodeCard_->width() / 2, r.bottom() + 8);
    at.setX(std::clamp(at.x(), 8, std::max(8, area.width() - nodeCard_->width() - 8)));
    at.setY(std::clamp(at.y(), 8, std::max(8, area.height() - nodeCard_->height() - 8)));
    nodeCard_->move(at);
    nodeCard_->show();
    nodeCard_->raise();
}

void HelpGuide::updateTooltipTarget(Step step) {
    auto* eng = EngineBridge::instance();
    const bool dragging = editor_->isDraggingConnection();
    tipTarget_.clear();
    tipBody_.clear();
    tipDir_ = HelpArrow::Left;
    const QVector<GraphNode> nodes = eng->nodes();
    if (step == ConnectSourceToEffect) {
        for (const GraphNode& n : nodes) {
            if (!dragging && !n.isShader) { tipTarget_ = n.id; break; }
            if (dragging && n.isShader && n.typeRaw != kBlendType) { tipTarget_ = n.id; break; }
        }
        tipTitle_ = dragging ? HelpStrings::completeConnectionTitle : HelpStrings::connectSourceEffectTitle;
        tipBody_ = dragging ? HelpStrings::dragToEffectInstruction : HelpStrings::connectInstruction;
        tipDir_ = dragging ? HelpArrow::Right : HelpArrow::Left;
    } else if (step == BlendViaOutputs) {
        // Nodes whose output isn't connected yet.
        QSet<QString> starts;
        for (const GraphEdge& e : eng->edges()) starts.insert(e.startId);
        QStringList ids;
        for (const GraphNode& n : nodes)
            if (!(n.isShader && n.typeRaw == kBlendType) && !starts.contains(n.id)) ids << n.id;
        if (!dragging && !ids.isEmpty()) tipTarget_ = ids[0];
        if (dragging && ids.size() >= 2) tipTarget_ = ids[1];
        tipTitle_ = dragging ? HelpStrings::blendViaOutputsDrop : HelpStrings::blendViaOutputsTitle;
        if (!dragging) tipBody_ = HelpStrings::blendViaOutputsInstruction;
    } else if (step == OpenInspector) {
        tipTarget_ = blendId_;
        tipTitle_ = HelpStrings::openShaderInfoTitle;
        tipBody_ = HelpStrings::clickBlendInstruction;
        tipDir_ = HelpArrow::Up;
    }
}

void HelpGuide::showCards(Step step) {
    const QSize area = canvas_->size();
    card_ = nullptr;
    auto* card = new HelpCard(canvas_);
    switch (step) {
    case PlaceSource:
        card->title(HelpStrings::welcomeTitle)
            ->body(QString(HelpStrings::welcomeSubtitle) + "\n\n" + HelpStrings::startGuide + "\n\n" + HelpStrings::sourceDragInstruction);
        break;
    case PlaceEffect:
        card->title(HelpStrings::addEffectTitle)->body(HelpStrings::effectDragInstruction);
        break;
    case PlaceSecondSource:
        card->title(HelpStrings::addSecondSourceTitle)
            ->body(QString(HelpStrings::addSecondSourceInstruction) + "\n\n" + HelpStrings::webcamVideoInstruction);
        break;
    case EditBlendMode:
        card->title(HelpStrings::changeParametersTitle)->body(HelpStrings::blendModeInstruction);
        break;
    case EditAlpha:
        card->title(HelpStrings::changeParametersTitle)
            ->body(QString(HelpStrings::alphaModeInstruction) + "\n\n" + HelpStrings::otherNodesInstruction);
        break;
    case ActionButtonsTour:
        delete card;
        card = new HelpCard(canvas_, 360);
        card->title(HelpStrings::actionButtonsTitle)
            ->iconRow(Icons::Undo, HelpStrings::undoInstruction)
            ->iconRow(Icons::Redo, HelpStrings::redoInstruction)
            ->iconRow(Icons::Camera, HelpStrings::cameraViewInstruction)
            ->iconRow(Icons::Help, HelpStrings::helpToggleInstruction)
            ->iconRow(Icons::Clear, HelpStrings::clearCanvasInstruction)
            ->iconRow(Icons::Fit, HelpStrings::scaleToFitInstruction)
            ->button("Okay!", [this] { seenActions_ = true; }, true);
        break;
    case ShareHelp:
        delete card;
        card = new HelpCard(canvas_, 400);
        card->title(HelpStrings::shareStrandTitle)
            ->body(HelpStrings::shareStrandDescription)
            ->bullet(HelpStrings::shareStrandStep1)
            ->bullet(HelpStrings::shareStrandStep2)
            ->bullet(HelpStrings::shareStrandStep3)
            ->bullet(HelpStrings::shareStrandStep4)
            ->body(HelpStrings::shareStrandNote, Theme::textTertiary, 8)
            ->button("Got it!", [this] { seenShare_ = true; });
        break;
    case Completion:
        delete card;
        card = new HelpCard(canvas_, 440);
        card->title(HelpStrings::completionTitle, 16)
            ->body(HelpStrings::completionDescription, Theme::textSecondary, 10)
            ->body(HelpStrings::completionDescription2, Theme::textSecondary, 10)
            ->body(HelpStrings::completionDescription3, Theme::textTertiary, 9)
            ->title(HelpStrings::completionDescription4)
            ->divider()
            ->button("Let me in!", [this] { setCompleted(true); }, true, true);
        break;
    default:
        delete card;
        return;
    }
    if (step == PlaceSource || step == PlaceEffect || step == PlaceSecondSource) card->arrow(HelpArrow::Left);
    if (step == EditBlendMode || step == EditAlpha) card->arrow(HelpArrow::Right);
    card->finish();
    switch (step) {
    case PlaceSource: case PlaceEffect: case PlaceSecondSource:
        card->move(std::max(8, 200 - card->width() / 2), area.height() / 2 - card->height() / 2);
        break;
    case ActionButtonsTour:
        card->move(area.width() - card->width() - 16, area.height() - card->height() - 64);
        break;
    case ShareHelp: case Completion:
        card->move((area.width() - card->width()) / 2, (area.height() - card->height()) / 2);
        break;
    default:
        break;  // tick() places the inspector cards
    }
    card_ = card;
}
