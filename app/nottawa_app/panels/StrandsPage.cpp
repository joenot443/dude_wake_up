#include "StrandsPage.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "TileGridView.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

StrandsPage::StrandsPage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Local / Community segmented control.
    auto* segment = new QWidget(this);
    segment->setObjectName("Segment");
    segment->setStyleSheet(QString(R"(
        #Segment { background: %1; border-radius: 6px; }
        #Segment QToolButton { border-radius: 5px; padding: 3px 0; color: %2; }
        #Segment QToolButton:checked { background: %3; color: %4; font-weight: 600; }
    )").arg(Theme::backgroundTertiary.name(), Theme::textSecondary.name(), Theme::surfacePressed.name(),
            Theme::textPrimary.name()));
    auto* segmentLayout = new QHBoxLayout(segment);
    segmentLayout->setContentsMargins(2, 2, 2, 2);
    segmentLayout->setSpacing(2);
    auto* group = new QButtonGroup(this);
    for (const char* label : {"Local", "Community"}) {
        auto* b = new QToolButton(segment);
        b->setText(label);
        b->setCheckable(true);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setCursor(Qt::PointingHandCursor);
        group->addButton(b);
        segmentLayout->addWidget(b);
        (QString(label) == "Local" ? localButton_ : communityButton_) = b;
    }
    localButton_->setChecked(true);
    connect(communityButton_, &QToolButton::toggled, this, &StrandsPage::setCommunity);
    auto* segmentRow = new QHBoxLayout;
    segmentRow->setContentsMargins(10, 6, 10, 6);
    segmentRow->addWidget(segment);
    layout->addLayout(segmentRow);

    modes_ = new QStackedWidget(this);
    layout->addWidget(modes_, 1);

    // Local strands.
    localPage_ = new QWidget(modes_);
    auto* local = new QVBoxLayout(localPage_);
    local->setContentsMargins(0, 0, 0, 0);
    local->setSpacing(0);
    auto* controls = new QHBoxLayout;
    controls->setContentsMargins(8, 2, 10, 4);
    controls->setSpacing(6);
    searchButton_ = new QToolButton(localPage_);
    searchButton_->setIcon(Icons::icon(Icons::Search, Theme::textSecondary, Theme::accent));
    searchButton_->setIconSize(QSize(14, 14));
    searchButton_->setCheckable(true);
    searchButton_->setAutoRaise(true);
    searchButton_->setToolTip("Search");
    count_ = new QLabel(localPage_);
    count_->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    controls->addWidget(searchButton_);
    controls->addWidget(count_);
    controls->addStretch(1);
    local->addLayout(controls);

    searchRow_ = new QWidget(localPage_);
    auto* searchLayout = new QHBoxLayout(searchRow_);
    searchLayout->setContentsMargins(10, 0, 10, 6);
    search_ = new QLineEdit(searchRow_);
    search_->setPlaceholderText("Search strands...");
    search_->setClearButtonEnabled(true);
    searchLayout->addWidget(search_);
    searchRow_->hide();
    local->addWidget(searchRow_);
    connect(searchButton_, &QToolButton::toggled, this, [this](bool on) {
        searchRow_->setVisible(on);
        if (on) search_->setFocus(); else search_->clear();
    });
    connect(search_, &QLineEdit::textChanged, this, &StrandsPage::rebuild);

    auto* line = new QFrame(localPage_);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    local->addWidget(line);

    grid_ = new TileGridView(nullptr, localPage_);
    grid_->setActivateOnClick(true);
    connect(grid_, &TileGridView::activated, this, [this](const TileItem& tile) {
        emit loadRequested(tile.payload.section(':', 1));
    });
    connect(grid_, &TileGridView::contextMenuRequested, this, [this](const TileItem& tile, QPoint pos) {
        const QString id = tile.payload.section(':', 1);
        const QString name = tile.name;
        QMenu menu(this);
        menu.addAction("Rename...", this, [this, id, name] {
            bool ok = false;
            const QString newName = QInputDialog::getText(this, "Rename Strand", "Enter a new name for this strand.",
                                                          QLineEdit::Normal, name, &ok).trimmed();
            if (ok && !newName.isEmpty() && newName != name) EngineBridge::instance()->renameStrand(id, newName);
        });
        menu.addSeparator();
        menu.addAction("Delete", this, [this, id, name] {
            const auto answer = QMessageBox::question(this, "Delete Strand?",
                QString("\"%1\" will be permanently deleted.").arg(name),
                QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Cancel);
            if (answer == QMessageBox::Yes) EngineBridge::instance()->deleteStrand(id);
        });
        menu.exec(pos);
    });
    local->addWidget(grid_, 1);
    modes_->addWidget(localPage_);

    setCommunityPage(nullptr);
    connect(EngineBridge::instance(), &EngineBridge::strandsChanged, this, &StrandsPage::reload);
}

void StrandsPage::setCommunityPage(QWidget* page) {
    if (!page) {
        auto* label = new QLabel("Community strands aren't available on Windows yet.", modes_);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(QString("color: %1; padding: 16px;").arg(Theme::textTertiary.name()));
        page = label;
    }
    if (communityPage_) { modes_->removeWidget(communityPage_); communityPage_->deleteLater(); }
    communityPage_ = page;
    modes_->addWidget(page);
    setCommunity(communityButton_->isChecked());
}

void StrandsPage::showCommunity() { communityButton_->setChecked(true); }

void StrandsPage::setCommunity(bool on) {
    modes_->setCurrentWidget(on ? communityPage_ : localPage_);
}

void StrandsPage::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    reload();
}

void StrandsPage::reload() {
    if (isVisible()) rebuild();
}

void StrandsPage::rebuild() {
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    const QVector<StrandItem> strands = engine->strands();
    count_->setText(QString("%1 strand%2").arg(strands.size()).arg(strands.size() == 1 ? "" : "s"));
    const QString text = search_->text().trimmed();
    TileSection section;
    for (const StrandItem& s : strands) {
        if (!text.isEmpty() && !s.name.contains(text, Qt::CaseInsensitive)) continue;
        TileItem t;
        t.name = s.name;
        t.payload = "strand:" + s.id;
        t.imagePath = s.imagePath;
        t.icon = Icons::Stack;
        t.tint = QColor(0xf0, 0x8c, 0x00);
        section.items.push_back(t);
    }
    grid_->setEmptyText(strands.isEmpty()
        ? "No strands saved yet.\n\nRight-click a node and choose\n\"Save Strand...\" to save one."
        : "No strands match your search.");
    grid_->setSections({section});
}
