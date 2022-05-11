#include "QGbmIntegration.h"
#include "QGbmScreen.h"
#include "QGbmWindow.h"
#include "QGbmOffscreenSurface.h"
#include "QGbmPlatform.h"

#include <qscreen.h>
#include <qdebug.h>

#include <private/qguiapplication_p.h>

#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtEglSupport/private/qeglplatformcontext_p.h>
#include <QtPlatformHeaders/qeglnativecontext.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformoffscreensurface.h>
#include <private/qinputdevicemanager_p_p.h>


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
            if( auto window = dynamic_cast< QGbmWindow* >( surface ) )
                return window->eglSurface();

            if( auto offscreenSurface = dynamic_cast< QGbmOffscreenSurface* >( surface ) )
                return offscreenSurface->eglSurface();

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

        void initialize() override
        {
            Inherited::initialize();
        }

        bool makeCurrent( QPlatformSurface* surface ) override
        {
            return Inherited::makeCurrent( surface );
        }

        void doneCurrent() override
        {
            Inherited::doneCurrent();
        }
    };
}

class QGbmIntegration::PrivateData
{
  public:
    QGenericUnixFontDatabase fontDatabase;

    QGbmScreen* screen = nullptr;
    QPlatformInputContext* inputContext = nullptr;
};

QGbmIntegration::QGbmIntegration()
    : m_data( new PrivateData() )
{
}

QGbmIntegration::~QGbmIntegration()
{
}

void QGbmIntegration::initialize()
{
    m_data->inputContext = QPlatformInputContextFactory::create();

    auto manager = QInputDeviceManagerPrivate::get(
        QGuiApplicationPrivate::inputDeviceManager() );

    manager->setDeviceCount( QInputDeviceManager::DeviceTypePointer, 1 );
    manager->setDeviceCount( QInputDeviceManager::DeviceTypeKeyboard, 1 );

    m_data->screen = new QGbmScreen( "offscreen", QSize( 2000, 2000 ) );
    QWindowSystemInterface::handleScreenAdded( m_data->screen, true );
}

void QGbmIntegration::destroy()
{
}

QPlatformOpenGLContext* QGbmIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context ) const
{
    PlatformContext* platformContext;

    auto eglDisplay = QGbm::eglDisplay();
    auto eglConfig = QGbm::eglConfig();

    if( context->nativeHandle().isNull() )
    {
        platformContext = new PlatformContext(
            context->format(), context->shareHandle(),
            eglDisplay, &eglConfig, QVariant() );

        context->setNativeHandle( QGbm::nativeContextHandle( platformContext ) );
    }
    else
    {
        platformContext = new PlatformContext( context->format(), context->shareHandle(),
            eglDisplay, nullptr, context->nativeHandle() );
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

QPlatformBackingStore* QGbmIntegration::createPlatformBackingStore( QWindow* ) const
{
    return nullptr;
}

QPlatformWindow* QGbmIntegration::createPlatformWindow( QWindow* window ) const
{
    return new QGbmWindow( window );
}

QPlatformOffscreenSurface* QGbmIntegration::createPlatformOffscreenSurface(
    QOffscreenSurface* surface ) const
{
    return new QGbmOffscreenSurface( surface );
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
