#include "QGbmSurface.h"
#include "QGbmWindow.h"
#include "QGbmIntegration.h"

#include <qpa/qwindowsysteminterface.h>

QGbmWindow::QGbmWindow( QWindow* window )
    : QPlatformWindow( window )
{
    const auto screenSize = window->screen()->size();

    if( window->windowState() == Qt::WindowFullScreen )
        Inherited::setGeometry( QRect( QPoint(), screenSize ) );

    m_surface = new QGbmSurface( screenSize );
}

QGbmWindow::~QGbmWindow()
{
    delete m_surface;
}

QSurfaceFormat QGbmWindow::format() const
{
    return m_surface->format();
}

void QGbmWindow::setGeometry( const QRect& rect )
{
    const auto oldRect = geometry();

    if( oldRect != rect )
    {
        Inherited::setGeometry( rect );
        QWindowSystemInterface::handleGeometryChange( window(), rect );
    }
}

void QGbmWindow::requestActivateWindow()
{
    Inherited::requestActivateWindow();

    QWindowSystemInterface::handleWindowActivated(
        window(), Qt::OtherFocusReason );
}

qreal QGbmWindow::devicePixelRatio() const
{
#if 0
    return screen()->geometry().width() / qreal( geometry().width() );
#else
    return Inherited::devicePixelRatio();
#endif
}

void* QGbmWindow::eglSurface() const
{
    return m_surface->eglSurface();
}
