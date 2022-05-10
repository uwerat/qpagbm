#include "QGbmScreen.h"
#include "QGbmIntegration.h"

#include <qsurfaceformat.h>
#include <qdebug.h>

#include <QtEglSupport/private/qeglconvenience_p.h>

#include <EGL/egl.h>
#include <gbm.h>

namespace
{
    inline EGLConfig eglConfiguration( EGLDisplay display )
    {
        QSurfaceFormat format;

        format.setDepthBufferSize( 24 );
        format.setStencilBufferSize( 8 );
        format.setRedBufferSize( 8 );
        format.setGreenBufferSize( 8 );
        format.setBlueBufferSize( 8 );
        format.setSamples( 4 );

        const auto config = q_configFromGLFormat( display, format );
    #if 0
        q_printEglConfig( display, config );
    #endif
        return config;
    }
}

QGbmScreen::QGbmScreen( void* gbmDevice, void* eglDisplay,
        const QString& name, const QSize& size )
    : m_name( name )
    , m_size( size )
{
    m_eglConfig = eglConfiguration( eglDisplay );

    auto gbmSurface = gbm_surface_create(
        static_cast< gbm_device* >( gbmDevice ), size.width(), size.height(),
        GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING );

    m_eglSurface = eglCreateWindowSurface( eglDisplay, m_eglConfig,
        reinterpret_cast< EGLNativeWindowType >( gbmSurface ), nullptr );

    Q_ASSERT( m_eglSurface != EGL_NO_SURFACE );

    qDebug() << "EGLSurface: " << m_eglSurface << size;
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

QPlatformCursor* QGbmScreen::cursor() const
{
#if 0
    #include <qpa/qplatformcursor.h>

    /*
        overriding changeCursor could be used to find out,
        when the cursor changes.
     */
    class Cursor final : public QPlatformCursor
    {
      public:
        void changeCursor( QCursor*, QWindow* ) override;
    };
#endif

    return Inherited::cursor();
}
