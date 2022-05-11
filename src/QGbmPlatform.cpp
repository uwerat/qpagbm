#include "QGbmPlatform.h"

#include <qdebug.h>
#include <qvariant.h>
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
            {
                qFatal( "Could not open egl display" );
            }

            EGLint major, minor;

            if( !eglInitialize( m_eglDisplay, &major, &minor ) )
            {
                qFatal( "Could not initialize egl display" );
            }

            m_eglConfig = eglConfiguration( m_eglDisplay );
        }

        ~Platform()
        {
            if( m_gbmDevice )
                gbm_device_destroy( m_gbmDevice );

            if( m_fd != -1 )
                qt_safe_close( m_fd );
        }

        int m_fd = -1;
        gbm_device* m_gbmDevice = nullptr;

        EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
        EGLConfig m_eglConfig = nullptr;
    };
}

Q_GLOBAL_STATIC( Platform, qgbmPlatform )

void* QGbm::gbmDevice()
{
    return qgbmPlatform ? qgbmPlatform->m_gbmDevice : nullptr;
}

void* QGbm::eglDisplay()
{
    return qgbmPlatform ? qgbmPlatform->m_eglDisplay : nullptr;
}

void* QGbm::eglConfig()
{
    return qgbmPlatform ? qgbmPlatform->m_eglConfig : nullptr;
}

QVariant QGbm::nativeContextHandle( const QEGLPlatformContext* context )
{
    const QEGLNativeContext nativeContext( context->eglContext(), eglDisplay() );
    return QVariant::fromValue< QEGLNativeContext >( nativeContext );
}
