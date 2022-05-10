#include "QGbmIntegration.h"
#include "QGbmScreen.h"
#include "QGbmWindow.h"
#include "QGbmFunctions.h"

#include <qscreen.h>

#include <qdebug.h>

#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtEglSupport/private/qeglplatformcontext_p.h>
#include <QtDeviceDiscoverySupport/private/qdevicediscovery_p.h>
#include <QtPlatformHeaders/qeglnativecontext.h>
#include <QtCore/private/qcore_unix_p.h>
#include <QtEglSupport/private/qeglconvenience_p.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <private/qinputdevicemanager_p_p.h>

#include <gbm.h>

namespace
{
    class PlatformContext : public QEGLPlatformContext
    {
        using Inherited = QEGLPlatformContext;

      public:
        PlatformContext( const QSurfaceFormat& format,
                QPlatformOpenGLContext* share, EGLDisplay display,
                EGLConfig* config, const QVariant& nativeHandle, Flags flags = Flags() )
            : QEGLPlatformContext( format, share, display, config, nativeHandle, flags )
        {
        }

        EGLSurface eglSurfaceForPlatformSurface( QPlatformSurface* surface ) override
        {
            if( auto window = dynamic_cast< QPlatformWindow* >( surface ) )
            {
                if( auto screen = dynamic_cast< QGbmScreen* >( window->screen() ) )
                {
                    return screen->eglSurface();
                }
            }

            return nullptr;
        }

        EGLSurface createTemporaryOffscreenSurface() override
        {
            qWarning() << "EGL/GBM does not support Pbuffers";
            return 0;
        }

        void destroyTemporaryOffscreenSurface( EGLSurface ) override
        {
            qWarning() << "EGL/GBM does not support Pbuffers";
        }
    };
}

class QGbmIntegration::PrivateData
{
  public:
    QGenericUnixFontDatabase fontDatabase;

    QGbmScreen* screen = nullptr;
    QPlatformInputContext* inputContext = nullptr;

    int fd = -1;
    gbm_device* gbmDevice = nullptr;
    EGLDisplay display = EGL_NO_DISPLAY;
};

QGbmIntegration::QGbmIntegration()
    : m_data( new PrivateData() )
{
}

QGbmIntegration::~QGbmIntegration()
{
}

void QGbmIntegration::initEGL()
{
    auto scanner = QDeviceDiscovery::create( QDeviceDiscovery::Device_VideoMask );

    const auto devices = scanner->scanConnectedDevices();
#if 1
    qDebug() << "Found the following video devices:" << devices;
#endif
    scanner->deleteLater();

    // /dev/dri/card0
    auto path = devices.first();

    m_data->fd = qt_safe_open( path.toLocal8Bit().constData(), O_RDWR | O_CLOEXEC );

    if( m_data->fd == -1 )
    {
        qErrnoWarning( "Could not open DRM device %s", qPrintable( path ) );
        return;
    }

    m_data->gbmDevice = gbm_create_device( m_data->fd );

    if( m_data->gbmDevice == nullptr )
    {
        qErrnoWarning( "Could not create GBM device" );
        qt_safe_close( m_data->fd );
        m_data->fd = -1;
    }

    gbm_device* gbmDevice = nullptr;

#if 1
    // nullptr in case of EGLFS ??
    gbmDevice = m_data->gbmDevice;
#endif

    m_data->display = eglGetDisplay( reinterpret_cast< EGLNativeDisplayType >( gbmDevice ) );

    if( m_data->display == EGL_NO_DISPLAY )
    {
        qFatal( "Could not open egl display" );
    }

    EGLint major, minor;

    if( !eglInitialize( m_data->display, &major, &minor ) )
    {
        qFatal( "Could not initialize egl display" );
    }
}

void QGbmIntegration::initialize()
{
    initEGL();

    m_data->inputContext = QPlatformInputContextFactory::create();

    auto manager = QInputDeviceManagerPrivate::get( QGbm::inputDeviceManager() );

    manager->setDeviceCount( QInputDeviceManager::DeviceTypePointer, 1 );
    manager->setDeviceCount( QInputDeviceManager::DeviceTypeKeyboard, 1 );

    m_data->screen = new QGbmScreen( m_data->gbmDevice, m_data->display,
        "offscreen", QSize( 2000, 2000 ) );

    QWindowSystemInterface::handleScreenAdded( m_data->screen, true );
}

void QGbmIntegration::destroy()
{
    if( m_data->gbmDevice )
    {
        gbm_device_destroy( m_data->gbmDevice );
        m_data->gbmDevice = nullptr;
    }

    if( m_data->fd != -1 )
    {
        qt_safe_close( m_data->fd );
        m_data->fd = -1;
    }
}

QPlatformOpenGLContext* QGbmIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context ) const
{
    PlatformContext* platformContext;

    if( context->nativeHandle().isNull() )
    {
        auto screen = dynamic_cast< const QGbmScreen* >( context->screen()->handle() );
        auto eglConfig = screen->eglConfig();

        platformContext = new PlatformContext(
            context->format(), context->shareHandle(),
            m_data->display, &eglConfig, QVariant() );

        const QEGLNativeContext nativeContext( platformContext->eglContext(), m_data->display );
        context->setNativeHandle( QVariant::fromValue< QEGLNativeContext >( nativeContext ) );
    }
    else
    {
        platformContext = new PlatformContext( context->format(), context->shareHandle(),
            m_data->display, nullptr, context->nativeHandle() );
    }

    return platformContext;
}

bool QGbmIntegration::hasCapability( QPlatformIntegration::Capability cap ) const
{
    switch( cap )
    {
        case OpenGL:
        case ThreadedOpenGL:
        case RasterGLSurface:
        case ThreadedPixmaps:
            return true;

        case WindowManagement:
            return false;

        default:
            return QPlatformIntegration::hasCapability( cap );
    }
}

QPlatformWindow* QGbmIntegration::createPlatformWindow( QWindow* window ) const
{
    return new QGbmWindow( window );
}

QPlatformBackingStore* QGbmIntegration::createPlatformBackingStore( QWindow* ) const
{
    return nullptr;
}

QAbstractEventDispatcher* QGbmIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformFontDatabase* QGbmIntegration::fontDatabase() const
{
    return &m_data->fontDatabase;
}

QPlatformInputContext* QGbmIntegration::inputContext() const
{
    return m_data->inputContext;
}
