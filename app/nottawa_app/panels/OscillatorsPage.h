#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QTimer>
#include <QWidget>

class OscillatorCard;
class QLabel;
class QVBoxLayout;

// The browser's Osc tab, after the macOS OscillatorsSidebarView: every
// enabled oscillator in the graph as a card with a live waveform and its
// controls. Clicking a card's header selects its node.
class OscillatorsPage : public QWidget {
    Q_OBJECT
public:
    explicit OscillatorsPage(QWidget* parent = nullptr);

signals:
    void nodeRequested(const QString& connectableId);

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    void refresh();

    QTimer       poll_;
    QWidget*     list_ = nullptr;
    QVBoxLayout* listLayout_ = nullptr;
    QWidget*     empty_ = nullptr;
    QHash<QString, OscillatorCard*> cards_;  // by param id
};
