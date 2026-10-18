#pragma once

// Design-system controls after the macOS DesignLibrary (DSSlider, DSToggle,
// DSPicker): painted with the theme tokens rather than the platform style.

#include <QStringList>
#include <QToolButton>
#include <QWidget>

class QLineEdit;

// A capsule slider with an optional label and value above it. Clicking the
// value lets you type one. valueChanged fires while dragging.
class DSSlider : public QWidget {
    Q_OBJECT
public:
    explicit DSSlider(QWidget* parent = nullptr);

    void setRange(float min, float max);
    void setStep(float step) { step_ = step; }  // 0: continuous
    void setLabel(const QString& label);
    void setShowValue(bool on);
    void setDecimals(int decimals) { decimals_ = decimals; update(); }
    void setValue(float value);  // doesn't emit
    float value() const { return value_; }
    bool isDragging() const { return dragging_ || editor_; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

signals:
    void valueChanged(float value);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    QRectF trackRect() const;
    QRect  valueRect() const;
    bool   hasHeader() const { return !label_.isEmpty() || showValue_; }
    void   setFromX(qreal x);
    void   editValue();

    float   min_ = 0, max_ = 1, value_ = 0, step_ = 0;
    int     decimals_ = 2;
    QString label_;
    bool    showValue_ = false;
    bool    dragging_ = false;
    QLineEdit* editor_ = nullptr;
};

// A switch (track + thumb) with an optional label to its left.
class DSToggle : public QWidget {
    Q_OBJECT
public:
    explicit DSToggle(const QString& label = {}, QWidget* parent = nullptr);
    void setChecked(bool on);  // doesn't emit
    bool isChecked() const { return checked_; }
    void setSmall(bool small) { small_ = small; updateGeometry(); update(); }

    QSize sizeHint() const override;

signals:
    void toggled(bool on);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    QString label_;
    bool    checked_ = false;
    bool    small_ = false;
};

// A borderless "Value ⌄" button that picks one of a list from a menu.
class DSPicker : public QToolButton {
    Q_OBJECT
public:
    explicit DSPicker(QWidget* parent = nullptr);
    void setOptions(const QStringList& options);
    void setCurrentIndex(int index);  // doesn't emit
    int  currentIndex() const { return index_; }

signals:
    void currentIndexChanged(int index);

private:
    QStringList options_;
    int index_ = -1;
};
