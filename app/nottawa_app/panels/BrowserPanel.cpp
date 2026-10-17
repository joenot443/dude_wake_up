#include "BrowserPanel.h"
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {
QTreeWidgetItem* addCategory(QTreeWidget* tree, const QString& name) {
    auto* top = new QTreeWidgetItem(tree, QStringList{name});
    QFont f = top->font(0); f.setBold(true); top->setFont(0, f);
    top->setFlags(Qt::ItemIsEnabled);   // non-selectable header
    top->setExpanded(true);
    return top;
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
    tree_->setRootIsDecorated(false);
    tree_->setAlternatingRowColors(true);
    layout->addWidget(tree_, /*stretch=*/1);
    // itemClicked for mouse, itemActivated for keyboard Enter.
    connect(tree_, &QTreeWidget::itemClicked,   this, &BrowserPanel::onItemActivated);
    connect(tree_, &QTreeWidget::itemActivated, this, &BrowserPanel::onItemActivated);
}

void BrowserPanel::populate(const QStringList& shaderNames) {
    tree_->clear();
    addCategory(tree_, "SOURCES")->addChild(new QTreeWidgetItem(QStringList{"(not yet available)"}));

    auto* effects = addCategory(tree_, "EFFECTS");
    for (const QString& n : shaderNames) {
        auto* leaf = new QTreeWidgetItem(effects, QStringList{n});
        leaf->setData(0, Qt::UserRole, n);  // only leaves carry a name, so headers never emit
    }

    addCategory(tree_, "STRANDS")->addChild(new QTreeWidgetItem(QStringList{"— none loaded —"}));
}

void BrowserPanel::setCurrentShader(const QString& name) {
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        auto* top = tree_->topLevelItem(i);
        for (int j = 0; j < top->childCount(); ++j) {
            auto* child = top->child(j);
            if (child->data(0, Qt::UserRole).toString() == name) {
                tree_->setCurrentItem(child);
                return;
            }
        }
    }
}

void BrowserPanel::onItemActivated(QTreeWidgetItem* item, int) {
    if (!item) return;
    const QString name = item->data(0, Qt::UserRole).toString();
    if (!name.isEmpty()) emit shaderPicked(name);
}

void BrowserPanel::onFilterChanged(const QString& text) {
    const QString needle = text.trimmed().toLower();
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        auto* top = tree_->topLevelItem(i);
        int visible = 0;
        for (int j = 0; j < top->childCount(); ++j) {
            auto* child = top->child(j);
            const bool hit = needle.isEmpty()
                || child->text(0).toLower().contains(needle);
            child->setHidden(!hit);
            if (hit) ++visible;
        }
        top->setHidden(!needle.isEmpty() && visible == 0);
    }
}
