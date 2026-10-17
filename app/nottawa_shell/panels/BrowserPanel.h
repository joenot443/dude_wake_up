#pragma once
#include <QWidget>
class QTreeWidget;
class QLineEdit;

// BrowserPanel — left sidebar. Phase A.1 is a scaffolded shell:
// placeholder tree with Shaders/Sources/Strands categories, each
// populated with a few mock entries. Phase A.3 wires it to the engine's
// real library (LibraryService::shaderCategories()).
class BrowserPanel : public QWidget {
    Q_OBJECT
public:
    explicit BrowserPanel(QWidget* parent = nullptr);
private:
    QLineEdit*   search_ = nullptr;
    QTreeWidget* tree_   = nullptr;
};
