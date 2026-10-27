#include "OscillatorsPage.h"

#include "Icons.h"
#include "Theme.h"
#include "ui/Controls.h"
#include "ui/WaveformPreview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QVBoxLayout>

// One oscillator: header, waveform, on/off and shape, frequency/min/max.
class OscillatorCard : public QWidget {
public:
    OscillatorCard(OscillatorsPage* page, const OscillatorInfo& info, QWidget* parent)
        : QWidget(parent), page_(page), paramId_(info.paramId), connectableId_(info.connectableId) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(8);

        header_ = new QLabel(this);
        header_->setCursor(Qt::PointingHandCursor);
        header_->setTextFormat(Qt::RichText);
        header_->setToolTip("Select this node");
        header_->installEventFilter(this);
        layout->addWidget(header_);

        wave_ = new WaveformPreview(this);
        layout->addWidget(wave_);

        auto* row = new QHBoxLayout;
        toggle_ = new DSToggle("Osc", this);
        toggle_->setSmall(true);
        connect(toggle_, &DSToggle::toggled, this, [this](bool on) {
            EngineBridge::instance()->setOscillatorEnabled(paramId_, on);
            setOn(on);
        });
        shape_ = new DSPicker(this);
        shape_->setOptions(WaveformPreview::shapeNames());
        shape_->setFixedWidth(100);
        connect(shape_, &DSPicker::currentIndexChanged, this, [this](int shape) {
            EngineBridge::instance()->setOscillatorWaveShape(paramId_, shape);
            wave_->setShape(shape);
        });
        row->addWidget(toggle_);
        row->addStretch(1);
        row->addWidget(shape_);
        layout->addLayout(row);

        frequency_ = new DSSlider(this);
        frequency_->setLabel("Frequency");
        frequency_->setShowValue(true);
        frequency_->setRange(0, 12);
        connect(frequency_, &DSSlider::valueChanged, this, [this](float f) {
            EngineBridge::instance()->setOscillatorFrequency(paramId_, f);
            wave_->setFrequency(f);
        });
        min_ = new DSSlider(this);
        min_->setLabel("Min");
        min_->setShowValue(true);
        max_ = new DSSlider(this);
        max_->setLabel("Max");
        max_->setShowValue(true);
        auto sendRange = [this] {
            EngineBridge::instance()->setOscillatorRange(paramId_, min_->value(), max_->value());
            wave_->setOutputRange(min_->value(), max_->value());
        };
        connect(min_, &DSSlider::valueChanged, this, sendRange);
        connect(max_, &DSSlider::valueChanged, this, sendRange);
        layout->addWidget(frequency_);
        layout->addWidget(min_);
        layout->addWidget(max_);
        setInfo(info);
    }

    // Takes the engine's state, except for a control being dragged.
    void setInfo(const OscillatorInfo& info) {
        connectableId_ = info.connectableId;
        header_->setText(QString("<span style='color:%1'>●</span>&nbsp; <b style='color:%2'>%3</b>"
                                 " <span style='color:%4'>on</span> <span style='color:%5'>%6</span>")
                             .arg((info.enabled ? Theme::accent : Theme::textTertiary).name(),
                                  Theme::textPrimary.name(), info.paramName.toHtmlEscaped(),
                                  Theme::textTertiary.name(), Theme::textSecondary.name(),
                                  info.ownerName.toHtmlEscaped()));
        toggle_->setChecked(info.enabled);
        shape_->setCurrentIndex(info.waveShape);
        frequency_->setValue(info.frequency);
        min_->setRange(info.paramMin, info.paramMax);
        max_->setRange(info.paramMin, info.paramMax);
        if (!min_->isDragging() && !max_->isDragging()) {
            min_->setValue(info.minOutput);
            max_->setValue(info.maxOutput);
        }
        wave_->setShape(info.waveShape);
        wave_->setFrequency(frequency_->value());
        wave_->setOutputRange(min_->value(), max_->value());
        wave_->setParamRange(info.paramMin, info.paramMax);
        setOn(info.enabled);
    }

protected:
    bool eventFilter(QObject* watched, QEvent* e) override {
        if (watched == header_ && e->type() == QEvent::MouseButtonRelease)
            emit page_->nodeRequested(connectableId_);
        return QWidget::eventFilter(watched, e);
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath card;
        card.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
        p.fillPath(card, Theme::surface);
        QColor border = on_ ? Theme::accent : Theme::borderSubtle;
        if (on_) border.setAlphaF(0.3);
        p.setPen(QPen(border, 1));
        p.drawPath(card);
    }

private:
    void setOn(bool on) {
        on_ = on;
        wave_->setOn(on);
        shape_->setVisible(on);
        frequency_->setVisible(on);
        min_->setVisible(on);
        max_->setVisible(on);
        update();
    }

    OscillatorsPage* page_;
    QString          paramId_, connectableId_;
    QLabel*          header_;
    WaveformPreview* wave_;
    DSToggle*        toggle_;
    DSPicker*        shape_;
    DSSlider*        frequency_;
    DSSlider*        min_;
    DSSlider*        max_;
    bool             on_ = false;
};

OscillatorsPage::OscillatorsPage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* line = new QFrame(this);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    layout->addWidget(line);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_ = new QWidget(scroll);
    listLayout_ = new QVBoxLayout(list_);
    listLayout_->setContentsMargins(8, 8, 8, 8);
    listLayout_->setSpacing(10);
    listLayout_->addStretch(1);
    scroll->setWidget(list_);
    layout->addWidget(scroll, 1);

    empty_ = new QLabel(QString("<div style='color:%1'>No active oscillators</div>"
                                "<div style='color:%2; font-size:8pt'>Enable an oscillator on any parameter<br>"
                                "to see it here</div>")
                            .arg(Theme::textTertiary.name(), Theme::textTertiary.darker(130).name()), this);
    static_cast<QLabel*>(empty_)->setAlignment(Qt::AlignCenter);
    empty_->setAttribute(Qt::WA_TransparentForMouseEvents);

    poll_.setInterval(500);
    connect(&poll_, &QTimer::timeout, this, &OscillatorsPage::refresh);
}

void OscillatorsPage::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    refresh();
    poll_.start();
}

void OscillatorsPage::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    poll_.stop();
}

void OscillatorsPage::refresh() {
    const QVector<OscillatorInfo> oscillators = EngineBridge::instance()->activeOscillators();
    QSet<QString> live;
    int index = 0;
    for (const OscillatorInfo& info : oscillators) {
        live.insert(info.paramId);
        OscillatorCard*& card = cards_[info.paramId];
        if (!card) card = new OscillatorCard(this, info, list_);
        else card->setInfo(info);
        if (listLayout_->indexOf(card) != index) {
            listLayout_->removeWidget(card);
            listLayout_->insertWidget(index, card);
        }
        ++index;
    }
    for (auto it = cards_.begin(); it != cards_.end();) {
        if (live.contains(it.key())) { ++it; continue; }
        it.value()->deleteLater();
        it = cards_.erase(it);
    }
    empty_->setGeometry(rect());
    empty_->setVisible(oscillators.isEmpty());
    empty_->raise();
}
