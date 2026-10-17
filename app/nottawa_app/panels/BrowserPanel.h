#pragma once
#include <QWidget>
#include <QString>
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;

// BrowserPanel — left sidebar. Scans the repo's shaders/ for real
// Nottawa shaders, groups them under EFFECTS. Clicking emits
// `shaderPicked` which MainWindow routes to the engine.
class BrowserPanel : public QWidget {
    Q_OBJECT
public:
    explicit BrowserPanel(QWidget* parent = nullptr);
    void populateFromDir(const QString& shadersDir);
    void setCurrentShader(const QString& name);

signals:
    void shaderPicked(const QString& name);

private slots:
    void onItemActivated(QTreeWidgetItem* item, int column);
    void onFilterChanged(const QString& text);

private:
    QLineEdit*   search_ = nullptr;
    QTreeWidget* tree_   = nullptr;
};
