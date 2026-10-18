#include "ParamSelection.h"

#include "Theme.h"

#include <QChildEvent>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QWidget>

namespace {

// The macOS selected-parameter look: an 8% accent wash and a 60% outline.
class Highlight : public QWidget {
public:
    explicit Highlight(QWidget* parent) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setObjectName("ParamHighlight");
        setStyleSheet("#ParamHighlight { background: transparent; }");
        hide();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath box;
        box.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
        QColor fill = Theme::accent, edge = Theme::accent;
        fill.setAlphaF(0.08);
        edge.setAlphaF(0.6);
        p.fillPath(box, fill);
        p.setPen(QPen(edge, 1));
        p.drawPath(box);
    }
};

// Watches one tracked widget and its descendants.
class Tracker : public QObject {
public:
    Tracker(QWidget* widget, const QString& paramId)
        : QObject(widget), widget_(widget), paramId_(paramId), highlight_(new Highlight(widget)) {
        watch(widget);
        auto* sel = ParamSelection::instance();
        connect(sel, &ParamSelection::changed, this, [this](const QString& id) { update(id); });
        update(sel->id());
    }

protected:
    bool eventFilter(QObject* watched, QEvent* e) override {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            ParamSelection::instance()->set(paramId_);
            break;
        case QEvent::ChildAdded:  // controls built after tracking began
            if (auto* child = qobject_cast<QWidget*>(static_cast<QChildEvent*>(e)->child()))
                if (child != highlight_) watch(child);
            break;
        case QEvent::Resize:
            if (watched == widget_) highlight_->setGeometry(widget_->rect());
            break;
        default:
            break;
        }
        return false;
    }

private:
    void watch(QWidget* w) {
        w->installEventFilter(this);
        for (QWidget* child : w->findChildren<QWidget*>())
            if (child != highlight_) child->installEventFilter(this);
    }
    void update(const QString& id) {
        const bool on = !paramId_.isEmpty() && id == paramId_;
        highlight_->setGeometry(widget_->rect());
        highlight_->setVisible(on);
        if (on) highlight_->raise();
    }

    QWidget*  widget_;
    QString   paramId_;
    Highlight* highlight_;
};

} // anon

ParamSelection* ParamSelection::instance() {
    static ParamSelection* selection = new ParamSelection;
    return selection;
}

void ParamSelection::set(const QString& paramId) {
    if (paramId == id_) return;
    id_ = paramId;
    emit changed(id_);
}

void ParamSelection::track(QWidget* widget, const QString& paramId) {
    if (widget) new Tracker(widget, paramId);
}
