#include "BrowserPanel.h"

#include <QLineEdit>
#include <functional>
#include <QMap>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {
constexpr int kTypeRole   = Qt::UserRole;
constexpr int kSourceRole = Qt::UserRole + 1;

QTreeWidgetItem* addHeader(QTreeWidget* tree, QTreeWidgetItem* parent, const QString& name) {
    auto* item = parent ? new QTreeWidgetItem(parent, QStringList{name})
                        : new QTreeWidgetItem(tree, QStringList{name});
    QFont f = item->font(0); f.setBold(true); item->setFont(0, f);
    item->setFlags(Qt::ItemIsEnabled);
    item->setExpanded(parent == nullptr);
    return item;
}

void addLeaves(QTreeWidgetItem* parent, const QVector<CatalogItem>& items) {
    for (const auto& c : items) {
        auto* leaf = new QTreeWidgetItem(parent, QStringList{c.name});
        leaf->setData(0, kTypeRole, c.typeRaw);
        leaf->setData(0, kSourceRole, c.isSource);
    }
}
} // anon

BrowserPanel::BrowserPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    search_ = new QLineEdit;
    search_->setPlaceholderText("Search library…");
    layout->addWidget(search_);
    connect(search_, &QLineEdit::textChanged, this, &BrowserPanel::onFilterChanged);

    tree_ = new QTreeWidget;
    tree_->setHeaderHidden(true);
    tree_->setAlternatingRowColors(true);
    layout->addWidget(tree_, 1);
    // itemClicked for mouse, itemActivated for keyboard Enter.
    connect(tree_, &QTreeWidget::itemClicked,   this, &BrowserPanel::onItemActivated);
    connect(tree_, &QTreeWidget::itemActivated, this, &BrowserPanel::onItemActivated);
}

void BrowserPanel::populate(const QVector<CatalogItem>& sources, const QVector<CatalogItem>& effects) {
    tree_->clear();
    addLeaves(addHeader(tree_, nullptr, "SOURCES"), sources);

    auto* fx = addHeader(tree_, nullptr, "EFFECTS");
    QMap<QString, QVector<CatalogItem>> byCategory;
    QStringList order;
    for (const auto& e : effects) {
        if (!byCategory.contains(e.category)) order << e.category;
        byCategory[e.category].push_back(e);
    }
    for (const QString& cat : order) addLeaves(addHeader(tree_, fx, cat), byCategory[cat]);
}

void BrowserPanel::onItemActivated(QTreeWidgetItem* item, int) {
    if (!item || !item->data(0, kTypeRole).isValid()) return;
    emit itemPicked(item->data(0, kTypeRole).toInt(), item->data(0, kSourceRole).toBool());
}

void BrowserPanel::onFilterChanged(const QString& text) {
    const QString needle = text.trimmed().toLower();
    // Returns whether anything under `item` is visible.
    std::function<bool(QTreeWidgetItem*)> apply = [&](QTreeWidgetItem* item) -> bool {
        if (item->data(0, kTypeRole).isValid()) {
            const bool hit = needle.isEmpty() || item->text(0).toLower().contains(needle);
            item->setHidden(!hit);
            return hit;
        }
        bool any = false;
        for (int i = 0; i < item->childCount(); ++i) any |= apply(item->child(i));
        item->setHidden(!any && !needle.isEmpty());
        if (!needle.isEmpty() && any) item->setExpanded(true);
        return any;
    };
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) apply(tree_->topLevelItem(i));
}
