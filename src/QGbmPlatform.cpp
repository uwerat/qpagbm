#include "QGbmPlatform.h"

#include <qdebug.h>
#include <qsurfaceformat.h>

#include <QtCore/private/qcore_unix_p.h>
#include <QtDeviceDiscoverySupport/private/qdevicediscovery_p.h>
#include <QtEglSupport/private/qeglplatformcontext_p.h>
#include <QtPlatformHeaders/qeglnativecontext.h>
#include <QtEglSupport/private/qeglconvenience_p.h>

#include <EGL/egl.h>
#include <gbm.h>

namespace
{
    #define CASE_STR( value ) case value: return #value;

    const char* getEglErrorString()
    {
        switch( eglGetError() )
        {
            CASE_STR( EGL_SUCCESS             )
            CASE_STR( EGL_NOT_INITIALIZED     )
            CASE_STR( EGL_BAD_ACCESS          )
            CASE_STR( EGL_BAD_ALLOC           )
            CASE_STR( EGL_BAD_ATTRIBUTE       )
            CASE_STR( EGL_BAD_CONTEXT         )
            CASE_STR( EGL_BAD_CONFIG          )
            CASE_STR( EGL_BAD_CURRENT_SURFACE )
            CASE_STR( EGL_BAD_DISPLAY         )
            CASE_STR( EGL_BAD_SURFACE         )
            CASE_STR( EGL_BAD_MATCH           )
            CASE_STR( EGL_BAD_PARAMETER       )
            CASE_STR( EGL_BAD_NATIVE_PIXMAP   )
            CASE_STR( EGL_BAD_NATIVE_WINDOW   )
            CASE_STR( EGL_CONTEXT_LOST        )
            default: return "Unknown";
        }
    }

    #undef CASE_STR

    EGLConfig eglConfiguration( EGLDisplay display )
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

    QString drmDevice()
    {
        auto scanner = QDeviceDiscovery::create( QDeviceDiscovery::Device_VideoMask );
        const auto devices = scanner->scanConnectedDevices();
        scanner->deleteLater();

        qDebug() << "Found the following video devices:" << devices;
        return devices.isEmpty() ? QString() : devices.first();
    }
}

namespace
{
    class Platform
    {
      public:
        Platform()
        {
            const auto path = drmDevice();
            if ( path.isEmpty() )
            {
                qFatal( "Could not find any DRM device." );
                return;
            }

            m_fd = qt_safe_open( path.toLocal8Bit().constData(), O_RDWR | O_CLOEXEC );

            if( m_fd == -1 )
            {
                qErrnoWarning( "Could not open DRM device %s", qPrintable( path ) );
                return;
            }

            m_gbmDevice = gbm_create_device( m_fd );

            if( m_gbmDevice == nullptr )
            {
                qErrnoWarning( "Could not create GBM device" );
                qt_safe_close( m_fd );
                m_fd = -1;
            }

            gbm_device* gbmDevice = nullptr;

        #if 1
            // nullptr in case of EGLFS ??
            gbmDevice = m_gbmDevice;
        #endif

            m_eglDisplay = eglGetDisplay(
                reinterpret_cast< EGLNativeDisplayType >( gbmDevice ) );

            if( m_eglDisplay == EGL_NO_DISPLAY )
                qFatal( "Could not open egl display" );

            EGLint major, minor;

            if( !eglInitialize( m_eglDisplay, &major, &minor ) )
                qFatal( "Could not initialize egl display" );

            m_eglConfig = eglConfiguration( m_eglDisplay );

            m_format = q_glFormatFromConfig( m_eglDisplay, m_eglConfig );
        }

        ~Platform()
        {
            if( m_gbmDevice )
                gbm_device_destroy( m_gbmDevice );

            if( m_fd != -1 )
                qt_safe_close( m_fd );
        }

        inline gbm_device* gbmDevice() const { return m_gbmDevice; }
        inline EGLDisplay eglDisplay() const { return m_eglDisplay; }
        inline EGLConfig eglConfig() const { return m_eglConfig; }

        inline QSurfaceFormat surfaceFormat() const { return m_format; }

      private:
        int m_fd = -1;
        gbm_device* m_gbmDevice = nullptr;

        EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
        EGLConfig m_eglConfig = nullptr;

        QSurfaceFormat m_format;
    };
}

Q_GLOBAL_STATIC( Platform, qgbm )

void* QGbm::eglDisplay()
{
    return qgbm ? qgbm->eglDisplay() : nullptr;
}

void* QGbm::eglConfig()
{
    return qgbm ? qgbm->eglConfig() : nullptr;
}

void* QGbm::createGbmSurface( int width, int height )
{
    return gbm_surface_create( qgbm->gbmDevice(),
        width, height, GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING );
}

void* QGbm::createEglSurface( void* gbmSurface )
{
    const auto native = reinterpret_cast< EGLNativeWindowType >( gbmSurface );
    auto eglSurface = eglCreateWindowSurface(
        qgbm->eglDisplay(), qgbm->eglConfig(), native, nullptr );

    Q_ASSERT( eglSurface != EGL_NO_SURFACE );
    if ( eglSurface == EGL_NO_SURFACE )
        qDebug() << "eglCreateWindowSurface:" << getEglErrorString();

    return eglSurface;
}

void QGbm::destroySurfaces( void* gbmSurface, void* eglSurface )
{
    if ( qgbm )
    {
        eglDestroySurface( qgbm->eglDisplay(), eglSurface );
        gbm_surface_destroy( reinterpret_cast< gbm_surface* >( gbmSurface ) );
    }
}

QSurfaceFormat QGbm::surfaceFormat()
{
    return qgbm ? qgbm->surfaceFormat() : QSurfaceFormat();
}
