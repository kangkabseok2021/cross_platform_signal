#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include "../ct_core/include/Types.h"

class CtImageProvider : public QQuickImageProvider {
public:
    CtImageProvider();

    // Called by QML Image { source: "image://ct/slice" }
    QImage requestImage(const QString& id, QSize* size,
                        const QSize& requestedSize) override;

    // Thread-safe update from reconstruction thread
    void updateImage(const ct::Image& img);

private:
    QMutex  m_mutex;
    QImage  m_current;
};
