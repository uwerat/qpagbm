#include "QGbmScreen.h"
#include "QGbmIntegration.h"

QGbmScreen::QGbmScreen( const QString& name, const QSize& size )
    : m_name( name )
    , m_size( size )
{
}

QString QGbmScreen::name() const
{
    return m_name;
}

QRect QGbmScreen::geometry() const
{
    return QRect( 0, 0, m_size.width(), m_size.height() );
}

int QGbmScreen::depth() const
{
    return 32;
}

QImage::Format QGbmScreen::format() const
{
    return QImage::Format_RGB32;
}

qreal QGbmScreen::refreshRate() const
{
    return Inherited::refreshRate();
}
