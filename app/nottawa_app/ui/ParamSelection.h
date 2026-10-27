#pragma once

#include <QObject>
#include <QString>

class QWidget;

// The parameter picked for keyboard control, after the macOS
// selectedParameterId: clicking a parameter's control selects it (with an
// accent highlight), Tab / Shift+Tab move through the inspector's or stage's
// parameters, Up/Down step it and Escape clears it (see MainWindow).
class ParamSelection : public QObject {
    Q_OBJECT
public:
    static ParamSelection* instance();

    const QString& id() const { return id_; }
    void set(const QString& paramId);
    void clear() { set({}); }

    // Highlights `widget` while `paramId` is selected, and selects it when
    // anything inside it is clicked.
    void track(QWidget* widget, const QString& paramId);

signals:
    void changed(const QString& paramId);

private:
    QString id_;
};
