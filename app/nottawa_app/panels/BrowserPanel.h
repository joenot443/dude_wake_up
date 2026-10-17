#pragma once
#include <QWidget>
#include <QString>
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;

// Left sidebar: Nottawa shaders under EFFECTS; clicking emits shaderPicked.
class BrowserPanel : public QWidget {
    Q_OBJECT
public:
    explicit BrowserPanel(QWidget* parent = nullptr);
    void populate(const QStringList& shaderNames);
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
