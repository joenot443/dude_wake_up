#pragma once

#include <QDialog>
#include <QImage>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

// Publishes a node's chain to the nottawa.app community, after the macOS
// ShareStrandSheet: preview, title, description, author; then the link.
class ShareStrandDialog : public QDialog {
    Q_OBJECT
public:
    ShareStrandDialog(const QString& nodeId, const QString& nodeName, const QImage& preview, QWidget* parent);

signals:
    void shared(const QString& slug);

private:
    void share();
    void finished(bool ok, const QString& slugOrError);

    QString         nodeId_;
    QImage          preview_;
    QLineEdit*      title_ = nullptr;
    QPlainTextEdit* description_ = nullptr;
    QLineEdit*      author_ = nullptr;
    QLabel*         status_ = nullptr;
    QPushButton*    shareButton_ = nullptr;
    QPushButton*    copyButton_ = nullptr;
    QString         link_;
    bool            busy_ = false;
};
