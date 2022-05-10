#pragma once

#include <qpa/qplatformscreen.h>
#include <qstring.h>
#include <qsize.h>

class QGbmScreen : public QPlatformScreen
{
    using Inherited = QPlatformScreen;

  public:
    QGbmScreen( void* gbmDevice, void* eglDisplay, const QString&, const QSize& );

    QString name() const override;
    QRect geometry() const override;

    int depth() const override;
    qreal refreshRate() const override;

    QImage::Format format() const override;

    QPlatformCursor* cursor() const override;

    void* eglSurface() const;
    void* eglConfig() const;

  private:
    void* m_eglSurface = nullptr;
    void* m_eglConfig = nullptr;

    const QString m_name;
    const QSize m_size;
};

inline void* QGbmScreen::eglSurface() const
{
    return m_eglSurface;
}

inline void* QGbmScreen::eglConfig() const
{
    return m_eglConfig;
}
