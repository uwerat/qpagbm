#include "QGbmSurface.h"
#include "QGbmOffscreenSurface.h"
#include "QGbmIntegration.h"

QGbmOffscreenSurface::QGbmOffscreenSurface(
        QOffscreenSurface* offscreenSurface )
    : QPlatformOffscreenSurface( offscreenSurface )
{
    m_surface = new QGbmSurface( QSize( 1, 1 ) );
}

QGbmOffscreenSurface::~QGbmOffscreenSurface()
{
    delete m_surface;
}

QSurfaceFormat QGbmOffscreenSurface::format() const
{
    return m_surface->format();
}

bool QGbmOffscreenSurface::isValid() const
{
    return m_surface->eglSurface() != nullptr;
}
