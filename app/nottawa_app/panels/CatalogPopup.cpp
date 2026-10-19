#include "CatalogPopup.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "panels/PreviewPool.h"
#include "panels/TileGridView.h"

#include <QButtonGroup>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
const QStringList kEffectCategories = {"All", "Basic", "Filter", "Glitch", "Transform", "Mix", "Mask"};
} // anon

CatalogPopup::CatalogPopup(bool effects, std::function<void(int)> onPick, QWidget* parent)
    : QFrame(parent, Qt::Popup), effects_(effects), onPick_(std::move(onPick)) {
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("CatalogPopup");
    setStyleSheet(QString("#CatalogPopup { background: %1; border: 1px solid %2; border-radius: 8px; }")
                      .arg(Theme::backgroundSecondary.name(), Theme::border.name()));
    resize(300, 440);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    search_ = new QLineEdit(this);
    search_->setPlaceholderText("Search...");
    search_->setClearButtonEnabled(true);
    search_->addAction(Icons::icon(Icons::Search, Theme::textSecondary), QLineEdit::LeadingPosition);
    connect(search_, &QLineEdit::textChanged, this, &CatalogPopup::rebuild);
    layout->addWidget(search_);

    // Category pills, scrolling sideways if they don't fit.
    QStringList categories = {"All"};
    if (effects_) {
        categories = kEffectCategories;
    } else {
        QStringList found;
        for (const CatalogItem& item : EngineBridge::instance()->sources())
            if (!item.category.isEmpty() && !found.contains(item.category)) found << item.category;
        found.sort(Qt::CaseInsensitive);
        categories += found;
    }
    auto* pillScroll = new QScrollArea(this);
    pillScroll->setFrameShape(QFrame::NoFrame);
    pillScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pillScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pillScroll->setFixedHeight(26);
    auto* pills = new QWidget(pillScroll);
    auto* pillRow = new QHBoxLayout(pills);
    pillRow->setContentsMargins(0, 0, 0, 0);
    pillRow->setSpacing(4);
    auto* group = new QButtonGroup(this);
    const QString pillStyle = QString("QToolButton { background: %1; color: %2; border: none; border-radius: 10px;"
                                      " padding: 2px 9px; font-size: 8pt; }"
                                      "QToolButton:checked { background: %3; color: %4; }")
                                  .arg(Theme::surface.name(), Theme::textSecondary.name(), Theme::accent.name(),
                                       Theme::textOnAccent.name());
    for (const QString& c : categories) {
        auto* pill = new QToolButton(pills);
        pill->setText(c);
        pill->setCheckable(true);
        pill->setChecked(c == category_);
        pill->setCursor(Qt::PointingHandCursor);
        pill->setStyleSheet(pillStyle);
        group->addButton(pill);
        connect(pill, &QToolButton::clicked, this, [this, c] { category_ = c; rebuild(); });
        pillRow->addWidget(pill);
    }
    pillRow->addStretch(1);
    pillScroll->setWidget(pills);
    pillScroll->setWidgetResizable(true);
    layout->addWidget(pillScroll);

    pool_ = new PreviewPool(this);
    grid_ = new TileGridView(pool_, this);
    grid_->setActivateOnClick(true);
    grid_->setEmptyText("No matches");
    connect(grid_, &TileGridView::activated, this, [this](const TileItem& tile) {
        const int type = tile.payload.section(':', 1).toInt();
        auto pick = onPick_;
        close();
        pick(type);
    });
    layout->addWidget(grid_, 1);

    auto* cancel = new QPushButton("Cancel", this);
    connect(cancel, &QPushButton::clicked, this, &QWidget::close);
    layout->addWidget(cancel, 0, Qt::AlignRight);
    rebuild();
}

CatalogPopup::~CatalogPopup() { pool_->clear(); }

void CatalogPopup::rebuild() {
    const QString text = search_->text().trimmed();
    const auto items = effects_ ? EngineBridge::instance()->effects() : EngineBridge::instance()->sources();
    TileSection section;
    for (const CatalogItem& item : items) {
        if (category_ != "All" && item.category != category_) continue;
        if (!text.isEmpty() && !item.name.contains(text, Qt::CaseInsensitive)) continue;
        TileItem t;
        t.name = item.name;
        t.payload = item.payload();
        t.live = true;
        t.icon = effects_ ? Icons::Sparkles : Icons::Video;
        section.items.push_back(t);
    }
    grid_->setSections({section});
}

void CatalogPopup::popup(QPoint globalPos) {
    QRect r(globalPos, size());
    if (QScreen* screen = QGuiApplication::screenAt(globalPos)) {
        const QRect avail = screen->availableGeometry();
        if (r.right() > avail.right()) r.moveRight(avail.right());
        if (r.bottom() > avail.bottom()) r.moveBottom(avail.bottom());
        if (r.left() < avail.left()) r.moveLeft(avail.left());
        if (r.top() < avail.top()) r.moveTop(avail.top());
    }
    move(r.topLeft());
    show();
    search_->setFocus();
}
