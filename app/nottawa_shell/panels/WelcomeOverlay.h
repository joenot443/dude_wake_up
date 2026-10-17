#pragma once
#include <QWidget>

// WelcomeOverlay — semi-opaque banner shown over the stage until a
// strand is opened or created. Mirrors NottawaApp's WelcomeView.
class WelcomeOverlay : public QWidget {
    Q_OBJECT
public:
    explicit WelcomeOverlay(QWidget* parent = nullptr);

signals:
    void newStrandRequested();
    void openStrandRequested();

protected:
    void paintEvent(QPaintEvent* e) override;
};
