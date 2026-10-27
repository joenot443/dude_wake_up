#pragma once

#include <QHash>
#include <QImage>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QThreadPool>

// Live engine previews for browser tiles, keyed by catalog payload
// ("shader:N" / "source:N"). Creating one compiles a shader on the engine
// thread, so creation and destruction run one at a time on a worker thread
// and previews appear progressively. Previews no longer wanted are kept for
// a while (most recently wanted first) so scrolling back is instant.
//
// Also holds still snapshots for when live previews are off.
class PreviewPool : public QObject {
    Q_OBJECT
public:
    explicit PreviewPool(QObject* parent = nullptr);
    ~PreviewPool() override;

    // Replaces the set of previews that should exist, in priority order.
    void setWanted(const QStringList& keys);
    QString previewId(const QString& key) const { return ids_.value(key); }
    // Destroys every preview (e.g. before the engine stops).
    void clear();

    QImage snapshot(const QString& key) const { return snapshots_.value(key); }
    void   setSnapshot(const QString& key, const QImage& image) { snapshots_.insert(key, image); }
    void   clearSnapshots() { snapshots_.clear(); }

    static constexpr int kSpare = 8;  // unwanted previews kept alive

signals:
    // A preview finished creating (or was destroyed).
    void changed();

private:
    void pump();

    QThreadPool             worker_;
    QHash<QString, QString> ids_;       // key -> preview id
    QStringList             wanted_;
    QStringList             recent_;    // existing keys, most recently wanted first
    QString                 inFlight_;  // key being created
    QHash<QString, QImage>  snapshots_;
};
