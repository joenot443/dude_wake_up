#include "InspectorPanel.h"
#include <QLabel>
#include <QVBoxLayout>
#include <Qt>

InspectorPanel::InspectorPanel(QWidget* parent) : QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(10, 10, 10, 10);
    layout_->setSpacing(8);

    auto* empty = new QLabel("Select a node to inspect its parameters.");
    empty->setAlignment(Qt::AlignCenter);
    empty->setWordWrap(true);
    empty->setStyleSheet("color: #6b6e7c; font-style: italic; padding: 24px;");
    layout_->addWidget(empty);
    layout_->addStretch(1);
}
