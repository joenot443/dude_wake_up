#pragma once
#include "EngineBridge.h"
#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

// Left sidebar: the engine's source and effect catalog. Clicking an entry
// emits itemPicked with the engine type to add.
class BrowserPanel : public QWidget {
    Q_OBJECT
public:
    explicit BrowserPanel(QWidget* parent = nullptr);
    void populate(const QVector<CatalogItem>& sources, const QVector<CatalogItem>& effects);

signals:
    void itemPicked(int typeRaw, bool isSource);

private slots:
    void onItemActivated(QTreeWidgetItem* item, int column);
    void onFilterChanged(const QString& text);

private:
    QLineEdit*   search_ = nullptr;
    QTreeWidget* tree_   = nullptr;
};
