#include "WelcomeOverlay.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "panels/TileGridView.h"

#include <QCheckBox>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

namespace {
const char* const kEnabledKey = "welcome/enabled";

// Fades a label in while sliding it up a few pixels, after `delayMs`.
void introduce(QLabel* label, int delayMs, int rise) {
    auto* fade = new QGraphicsOpacityEffect(label);
    fade->setOpacity(0);
    label->setGraphicsEffect(fade);
    label->setContentsMargins(0, rise, 0, 0);
    auto* opacity = new QPropertyAnimation(fade, "opacity", label);
    opacity->setDuration(550);
    opacity->setEndValue(1.0);
    opacity->setEasingCurve(QEasingCurve::OutCubic);
    auto* slide = new QVariantAnimation(label);
    slide->setDuration(550);
    slide->setStartValue(rise);
    slide->setEndValue(0);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(slide, &QVariantAnimation::valueChanged, label,
                     [label](const QVariant& v) { label->setContentsMargins(0, v.toInt(), 0, 0); });
    QTimer::singleShot(delayMs, label, [opacity, slide] {
        opacity->start();
        slide->start();
    });
}
} // anon

// One of the three action cards: icon, title, subtitle. The primary card's
// icon and border are accent-coloured.
class WelcomeCard : public QWidget {
    Q_OBJECT
public:
    WelcomeCard(Icons::Kind icon, const QString& title, const QString& subtitle, QWidget* parent)
        : QWidget(parent), icon_(icon), title_(title), subtitle_(subtitle) {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        setMinimumHeight(64);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    void setPrimary(bool on) { primary_ = on; update(); }
    void setAvailable(bool on) {
        available_ = on;
        setCursor(on ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QSize sizeHint() const override { return {220, 64}; }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (!available_) p.setOpacity(0.45);
        const bool hovered = available_ && underMouse();
        QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
        if (hovered) {  // the macOS card grows 1.5% under the pointer
            const qreal dx = r.width() * 0.0075, dy = r.height() * 0.0075;
            r.adjust(-dx + 1, -dy + 1, dx - 1, dy - 1);
        }
        QPainterPath card;
        card.addRoundedRect(r, 10, 10);
        p.fillPath(card, hovered ? Theme::surfaceHover : Theme::surface);
        QColor border = primary_ ? Theme::accent : Theme::borderSubtle;
        if (primary_) border.setAlphaF(0.5);
        p.setPen(QPen(border, primary_ ? 1.0 : 0.5));
        p.drawPath(card);
        const QRect iconRect(int(r.left()) + 16, int(r.center().y()) - 14, 28, 28);
        Icons::icon(icon_, primary_ ? Theme::accent : Theme::textSecondary).paint(&p, iconRect.adjusted(3, 3, -3, -3));
        const int textX = iconRect.right() + 14;
        QFont f = font();
        f.setPixelSize(14);
        f.setWeight(QFont::DemiBold);
        p.setFont(f);
        p.setPen(Theme::textPrimary);
        p.drawText(QRect(textX, int(r.center().y()) - 19, width() - textX - 12, 20), Qt::AlignLeft | Qt::AlignVCenter, title_);
        f.setPixelSize(11);
        f.setWeight(QFont::Normal);
        p.setFont(f);
        p.setPen(Theme::textTertiary);
        const QRect sub(textX, int(r.center().y()) + 2, width() - textX - 12, 16);
        p.drawText(sub, Qt::AlignLeft | Qt::AlignVCenter, QFontMetrics(f).elidedText(subtitle_, Qt::ElideRight, sub.width()));
    }
    void mouseReleaseEvent(QMouseEvent* e) override {
        if (available_ && rect().contains(e->position().toPoint())) emit clicked();
    }

private:
    Icons::Kind icon_;
    QString title_, subtitle_;
    bool primary_ = false;
    bool available_ = true;
};

WelcomeOverlay::WelcomeOverlay(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);

    panel_ = new QWidget(this);
    panel_->setObjectName("WelcomePanel");
    panel_->setStyleSheet(QString("#WelcomePanel { background: %1; border-radius: 12px; }").arg(Theme::backgroundSecondary.name()));

    auto* layout = new QVBoxLayout(panel_);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* header = new QVBoxLayout;
    header->setContentsMargins(28, 32, 28, 32);
    header->setSpacing(8);
    title_ = new QLabel("Welcome to Nottawa.", panel_);
    title_->setStyleSheet(QString("color: %1; font-family: Georgia, 'Times New Roman', serif; font-size: 29pt; font-weight: 600;")
                              .arg(Theme::textPrimary.name()));
    subtitle_ = new QLabel("Make something cool.", panel_);
    subtitle_->setStyleSheet(QString("color: %1; font-family: Georgia, 'Times New Roman', serif; font-size: 13pt; font-style: italic;")
                                 .arg(Theme::textSecondary.name()));
    header->addStretch(1);
    header->addWidget(title_);
    header->addWidget(subtitle_);
    auto* headerBox = new QWidget(panel_);
    headerBox->setLayout(header);
    headerBox->setMinimumHeight(160);
    layout->addWidget(headerBox);

    auto* cards = new QHBoxLayout;
    cards->setContentsMargins(28, 0, 28, 0);
    cards->setSpacing(14);
    resume_ = new WelcomeCard(Icons::Undo, "Resume", "Continue where you left off", panel_);
    newCard_ = new WelcomeCard(Icons::NewSquare, "New Workspace", "Start with a blank canvas", panel_);
    auto* open = new WelcomeCard(Icons::Folder, "Open Workspace", "Pick a saved file", panel_);
    connect(resume_, &WelcomeCard::clicked, this, &QWidget::hide);
    connect(newCard_, &WelcomeCard::clicked, this, [this] { hide(); emit newRequested(); });
    connect(open, &WelcomeCard::clicked, this, [this] { emit openRequested(); });
    cards->addWidget(resume_);
    cards->addWidget(newCard_);
    cards->addWidget(open);
    layout->addLayout(cards);
    layout->addSpacing(24);

    // Templates | Recent.
    auto* browsers = new QHBoxLayout;
    browsers->setContentsMargins(16, 0, 16, 16);
    browsers->setSpacing(16);
    auto column = [this](const QString& heading, TileGridView*& grid) {
        auto* box = new QWidget(panel_);
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(8);
        auto* label = new QLabel(heading, box);
        label->setStyleSheet(QString("color: %1; font-size: 11pt; font-weight: 600; padding-left: 8px;").arg(Theme::textPrimary.name()));
        v->addWidget(label);
        grid = new TileGridView(nullptr, box);
        grid->setActivateOnClick(true);
        v->addWidget(grid, 1);
        connect(grid, &TileGridView::activated, this, [this](const TileItem& tile) {
            hide();
            emit strandPicked(tile.payload);
        });
        return box;
    };
    browsers->addWidget(column("Templates", templates_), 1);
    auto* divider = new QFrame(panel_);
    divider->setFixedWidth(1);
    divider->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    browsers->addWidget(divider);
    browsers->addWidget(column("Recent", recent_), 1);
    recent_->setEmptyText("No Recent Strands\nSaved strands will appear here.");
    layout->addLayout(browsers, 1);

    auto* always = new QCheckBox("Always show at launch", panel_);
    always->setChecked(enabledAtLaunch());
    always->setStyleSheet(QString("QCheckBox { color: %1; font-size: 8pt; }").arg(Theme::textTertiary.name()));
    auto* fade = new QGraphicsOpacityEffect(always);
    fade->setOpacity(0.6);
    always->setGraphicsEffect(fade);
    connect(always, &QCheckBox::toggled, this, [](bool on) { QSettings().setValue(kEnabledKey, on); });
    auto* footer = new QHBoxLayout;
    footer->setContentsMargins(20, 0, 20, 14);
    footer->addStretch(1);
    footer->addWidget(always);
    layout->addLayout(footer);
}

bool WelcomeOverlay::enabledAtLaunch() { return QSettings().value(kEnabledKey, true).toBool(); }

void WelcomeOverlay::present() {
    auto* eng = EngineBridge::instance();
    const bool hasSession = eng->isRunning() && !eng->nodes().isEmpty();
    resume_->setAvailable(hasSession);
    resume_->setPrimary(hasSession);
    newCard_->setPrimary(!hasSession);
    auto toSection = [](const QVector<StrandItem>& strands) {
        TileSection section;
        for (const StrandItem& s : strands) {
            TileItem t;
            t.name = s.name;
            t.payload = s.id;
            t.imagePath = s.imagePath;
            t.icon = Icons::Stack;
            section.items.push_back(t);
        }
        return section;
    };
    templates_->setSections({toSection(eng->templateStrands())});
    recent_->setSections({toSection(eng->strands())});
    introduce(title_, 0, 8);
    introduce(subtitle_, 200, 6);
    layoutPanel();
    show();
    raise();
    setFocus();
}

void WelcomeOverlay::layoutPanel() {
    const QSize s(int(width() * 0.6), int(height() * 0.8));
    panel_->setGeometry((width() - s.width()) / 2, (height() - s.height()) / 2, s.width(), s.height());
}

void WelcomeOverlay::resizeEvent(QResizeEvent*) { layoutPanel(); }

void WelcomeOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 128));
    // A soft shadow around the panel (a graphics effect would stop its tile
    // grids from painting).
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    const QRectF panel = panel_->geometry();
    for (int i = 20; i > 0; i -= 2) {
        p.setBrush(QColor(0, 0, 0, 6));
        p.drawRoundedRect(panel.adjusted(-i, -i, i, i), 12 + i, 12 + i);
    }
}

void WelcomeOverlay::mousePressEvent(QMouseEvent* e) {
    if (!panel_->geometry().contains(e->position().toPoint())) hide();
}

void WelcomeOverlay::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) hide();
    else QWidget::keyPressEvent(e);
}

#include "WelcomeOverlay.moc"
