#pragma once

#include <QTimer>
#include <QWidget>

class QGraphicsOpacityEffect;
class QPropertyAnimation;

// A transient message in a pill near the bottom of its parent, after the
// macOS ToastView. Mouse events pass through it.
class Toast : public QWidget {
    Q_OBJECT
public:
    explicit Toast(QWidget* parent);
    void show(const QString& message, int ms = 2500);

protected:
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    void place();

    QString message_;
    QTimer  hideTimer_;
    QGraphicsOpacityEffect* opacity_;
    QPropertyAnimation*     fade_;
};
