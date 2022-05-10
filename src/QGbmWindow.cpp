#include "QGbmWindow.h"

#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface.h>

QGbmWindow::QGbmWindow( QWindow* window )
    : QPlatformWindow( window )
{
    if( window->windowState() == Qt::WindowFullScreen )
    {
        const auto sz = window->screen()->size();
        Inherited::setGeometry( QRect( 0, 0, sz.width(), sz.height() ) );
    }
}

void QGbmWindow::setGeometry( const QRect& rect )
{
    const QRect oldRect = geometry();

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
