#include "PreviewPool.h"

#include "EngineBridge.h"

#include <QCoreApplication>
#include <QPointer>

namespace {

// "shader:N" / "source:N" -> (type, isSource); false for anything else.
bool parseKey(const QString& key, int* type, bool* isSource) {
    const int colon = key.indexOf(':');
    if (colon < 0) return false;
    const QStringView kind = QStringView(key).left(colon);
    if (kind != u"shader" && kind != u"source") return false;
    *isSource = kind == u"source";
    *type = key.mid(colon + 1).toInt();
    return true;
}

} // anon

PreviewPool::PreviewPool(QObject* parent) : QObject(parent) {
    worker_.setMaxThreadCount(1);
}

PreviewPool::~PreviewPool() {
    clear();
    worker_.waitForDone();
}

void PreviewPool::setWanted(const QStringList& keys) {
    wanted_.clear();
    for (const QString& key : keys) {
        int type; bool isSource;
        if (parseKey(key, &type, &isSource)) wanted_.push_back(key);
    }
    // Wanted keys move to the front of the recency list.
    for (auto it = wanted_.crbegin(); it != wanted_.crend(); ++it) {
        if (recent_.removeOne(*it) || ids_.contains(*it)) recent_.prepend(*it);
    }
    // Destroy unwanted previews beyond the spare allowance.
    int spare = 0;
    for (int i = 0; i < recent_.size();) {
        const QString key = recent_[i];
        if (wanted_.contains(key) || ++spare <= kSpare) { ++i; continue; }
        const QString id = ids_.take(key);
        recent_.removeAt(i);
        worker_.start([id] { EngineBridge::destroyPreview(id); });
    }
    pump();
}

void PreviewPool::pump() {
    if (!inFlight_.isEmpty() || !EngineBridge::instance()->isRunning()) return;
    for (const QString& key : std::as_const(wanted_)) {
        if (ids_.contains(key)) continue;
        int type; bool isSource;
        parseKey(key, &type, &isSource);
        inFlight_ = key;
        QPointer<PreviewPool> self(this);
        worker_.start([self, key, type, isSource] {
            const QString id = EngineBridge::createPreview(type, isSource);
            QMetaObject::invokeMethod(QCoreApplication::instance(), [self, key, id] {
                if (!self) { if (!id.isEmpty()) EngineBridge::destroyPreview(id); return; }
                self->inFlight_.clear();
                if (!id.isEmpty()) {
                    if (self->wanted_.contains(key)) {
                        self->ids_.insert(key, id);
                        self->recent_.prepend(key);
                        emit self->changed();
                    } else {
                        self->worker_.start([id] { EngineBridge::destroyPreview(id); });
                    }
                }
                self->pump();
            }, Qt::QueuedConnection);
        });
        return;
    }
}

void PreviewPool::clear() {
    wanted_.clear();
    recent_.clear();
    const QStringList ids = ids_.values();
    ids_.clear();
    if (!EngineBridge::instance()->isRunning()) return;
    for (const QString& id : ids) worker_.start([id] { EngineBridge::destroyPreview(id); });
    worker_.waitForDone();
    emit changed();
}
