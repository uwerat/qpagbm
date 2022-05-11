#pragma once

#include <qpa/qplatformscreen.h>
#include <qstring.h>
#include <qsize.h>

class QGbmScreen : public QPlatformScreen
{
    using Inherited = QPlatformScreen;

  public:
    QGbmScreen( const QString&, const QSize& );

    QString name() const override;
    QRect geometry() const override;

    int depth() const override;
    qreal refreshRate() const override;

    QImage::Format format() const override;

  private:
    const QString m_name;
    const QSize m_size;
};
