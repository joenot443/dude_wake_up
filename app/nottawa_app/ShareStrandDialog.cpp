#include "ShareStrandDialog.h"

#include "EngineBridge.h"
#include "Theme.h"

#include <QBuffer>
#include <QClipboard>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

ShareStrandDialog::ShareStrandDialog(const QString& nodeId, const QString& nodeName, const QImage& preview,
                                     QWidget* parent)
    : QDialog(parent), nodeId_(nodeId), preview_(preview) {
    setWindowTitle("Share Strand");
    setFixedWidth(420);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    // Preview, rounded.
    auto* image = new QLabel(this);
    const QSize size(380, 160);
    QPixmap pm(size * devicePixelRatioF());
    pm.setDevicePixelRatio(devicePixelRatioF());
    pm.fill(Qt::transparent);
    {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        QPainterPath shape;
        shape.addRoundedRect(QRectF(0, 0, size.width(), size.height()), 8, 8);
        p.fillPath(shape, Theme::backgroundTertiary);
        if (!preview.isNull()) {
            const QSizeF fit = QSizeF(preview.size()).scaled(size, Qt::KeepAspectRatio);
            const QRectF target((size.width() - fit.width()) / 2, (size.height() - fit.height()) / 2, fit.width(), fit.height());
            p.setClipPath(shape);
            p.drawImage(target, preview);
        }
    }
    image->setPixmap(pm);
    layout->addWidget(image);

    title_ = new QLineEdit(nodeName, this);
    title_->setPlaceholderText("Title (required)");
    description_ = new QPlainTextEdit(this);
    description_->setPlaceholderText("Description (optional)");
    description_->setFixedHeight(70);
    author_ = new QLineEdit(QSettings().value("community/author").toString(), this);
    author_->setPlaceholderText("Author (optional)");
    layout->addWidget(title_);
    layout->addWidget(description_);
    layout->addWidget(author_);

    auto* note = new QLabel("Shared strands are public on nottawa.app.", this);
    note->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textTertiary.name()));
    layout->addWidget(note);

    status_ = new QLabel(this);
    status_->setWordWrap(true);
    status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    status_->hide();
    layout->addWidget(status_);

    auto* buttons = new QHBoxLayout;
    auto* cancel = new QPushButton("Cancel", this);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    copyButton_ = new QPushButton("Copy Link", this);
    copyButton_->hide();
    connect(copyButton_, &QPushButton::clicked, this, [this] { QGuiApplication::clipboard()->setText(link_); });
    shareButton_ = new QPushButton("Share", this);
    shareButton_->setDefault(true);
    connect(shareButton_, &QPushButton::clicked, this, [this] { link_.isEmpty() ? share() : accept(); });
    buttons->addWidget(cancel);
    buttons->addStretch(1);
    buttons->addWidget(copyButton_);
    buttons->addWidget(shareButton_);
    layout->addLayout(buttons);

    auto updateEnabled = [this] { shareButton_->setEnabled(!busy_ && (!link_.isEmpty() || !title_->text().trimmed().isEmpty())); };
    connect(title_, &QLineEdit::textChanged, this, updateEnabled);
    updateEnabled();
    connect(EngineBridge::instance(), &EngineBridge::strandShared, this, &ShareStrandDialog::finished);
}

void ShareStrandDialog::share() {
    const QString author = author_->text().trimmed();
    if (!author.isEmpty()) QSettings().setValue("community/author", author);
    QByteArray png;
    if (!preview_.isNull()) {
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        preview_.save(&buffer, "PNG");
    }
    busy_ = true;
    shareButton_->setEnabled(false);
    status_->setStyleSheet(QString("color: %1;").arg(Theme::textSecondary.name()));
    status_->setText("Uploading...");
    status_->show();
    EngineBridge::instance()->shareStrand(nodeId_, title_->text().trimmed(), description_->toPlainText().trimmed(),
                                          author, png);
}

void ShareStrandDialog::finished(bool ok, const QString& slugOrError) {
    if (!busy_) return;
    busy_ = false;
    if (!ok) {
        status_->setStyleSheet(QString("color: %1;").arg(Theme::destructive.name()));
        status_->setText(slugOrError.isEmpty() ? "Sharing failed." : slugOrError);
        shareButton_->setEnabled(true);
        return;
    }
    link_ = "https://nottawa.app/strands/" + slugOrError;
    status_->setStyleSheet(QString("color: %1;").arg(Theme::success.name()));
    status_->setText("Shared successfully!\n" + link_);
    copyButton_->show();
    shareButton_->setText("Done");
    shareButton_->setEnabled(true);
    title_->setEnabled(false);
    description_->setEnabled(false);
    author_->setEnabled(false);
    emit shared(slugOrError);
}
