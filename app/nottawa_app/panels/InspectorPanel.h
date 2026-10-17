#pragma once
#include <QString>
#include <QWidget>

class QLabel;
class QVBoxLayout;

// Right sidebar: editable parameters of one engine node, built from
// ntw_get_parameters. Standard, int (incl. option lists), bool and text
// parameters get controls; colour editing is not yet supported.
class InspectorPanel : public QWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);
    void setNode(const QString& connectableId, const QString& name);
    QString node() const { return nodeId_; }

private:
    void rebuild();

    QString      nodeId_;
    QLabel*      title_  = nullptr;
    QWidget*     form_   = nullptr;
    QVBoxLayout* layout_ = nullptr;
};
