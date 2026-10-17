#include "BrowserPanel.h"
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {
void addCategory(QTreeWidget* tree, const QString& name,
                 const QStringList& children) {
    auto* top = new QTreeWidgetItem(tree, QStringList{name});
    QFont f = top->font(0); f.setBold(true); top->setFont(0, f);
    for (const auto& c : children) {
        new QTreeWidgetItem(top, QStringList{c});
    }
    top->setExpanded(true);
}
} // anon

BrowserPanel::BrowserPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    search_ = new QLineEdit;
    search_->setPlaceholderText("Search library…");
    layout->addWidget(search_);

    tree_ = new QTreeWidget;
    tree_->setHeaderHidden(true);
    tree_->setRootIsDecorated(false);
    tree_->setAlternatingRowColors(true);
    layout->addWidget(tree_, /*stretch=*/1);

    // Scaffolded content — categories & example entries so the panel
    // isn't an empty box. Real wiring to LibraryService lands in A.3.
    addCategory(tree_, "SOURCES",
        {"Webcam", "File", "Image", "Text", "Shader", "Library", "Multi"});
    addCategory(tree_, "EFFECTS",
        {"Plasma", "Pixelate", "Color Grade", "Chromatic Aberration",
         "Feedback", "Mirror", "Edge Detect", "Blur"});
    addCategory(tree_, "STRANDS",
        {"— no strand loaded —"});
}
