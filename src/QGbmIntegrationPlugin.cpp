#include "QGbmPlatform.h"

#include <qscreen.h>
#include <qdebug.h>

#include <qpa/qplatformintegrationplugin.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformoffscreensurface.h>

#include <private/qguiapplication_p.h>
#include <private/qinputdevicemanager_p_p.h>

#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>

#include <QtEglSupport/private/qeglplatformcontext_p.h>
#include <QtPlatformHeaders/qeglnativecontext.h>

namespace
{
    class GbmOffscreenSurface : public QPlatformOffscreenSurface
    {
      public:
        GbmOffscreenSurface( QOffscreenSurface* offscreenSurface )
            : QPlatformOffscreenSurface( offscreenSurface )
        {
            m_gbmSurface = QGbm::createGbmSurface( 1, 1 );
            m_eglSurface = QGbm::createEglSurface( m_gbmSurface );
        }

        ~GbmOffscreenSurface() override
        {
            QGbm::destroySurfaces( m_gbmSurface, m_eglSurface );
        }

        QSurfaceFormat format() const override {   return QGbm::surfaceFormat(); }
        bool isValid() const override { return m_eglSurface != nullptr; }

        void* eglSurface() const { return m_eglSurface; }

      private:
        void* m_gbmSurface = nullptr;
        void* m_eglSurface = nullptr;
    };
}

namespace
{
    class GbmWindow : public QPlatformWindow
    {
        using Inherited = QPlatformWindow;

      public:
        GbmWindow( QWindow* window )
            : QPlatformWindow( window )
        {
            const auto sz = window->screen()->size();

            if( window->windowState() == Qt::WindowFullScreen )
                Inherited::setGeometry( QRect( 0, 0, sz.width(), sz.height() ) );

            m_gbmSurface = QGbm::createGbmSurface( sz.width(), sz.height() );
            m_eglSurface = QGbm::createEglSurface( m_gbmSurface );
        }

        ~GbmWindow() override
        {
            QGbm::destroySurfaces( m_gbmSurface, m_eglSurface );
        }

        void setGeometry( const QRect& rect ) override
        {
            const auto oldRect = geometry();

            if( oldRect != rect )
            {
                Inherited::setGeometry( rect );
                QWindowSystemInterface::handleGeometryChange( window(), rect );
            }
        }

        void requestActivateWindow() override
        {
            Inherited::requestActivateWindow();

            QWindowSystemInterface::handleWindowActivated(
                window(), Qt::OtherFocusReason );
        }

        qreal devicePixelRatio() const override
        {
            if ( m_scaleToScreen )
                return screen()->geometry().width() / qreal( geometry().width() );

            return Inherited::devicePixelRatio();
        }

        QSurfaceFormat format() const override { return QGbm::surfaceFormat(); }
        void* eglSurface() const { return m_eglSurface; }

      private:
        void* m_gbmSurface = nullptr;
        void* m_eglSurface = nullptr;

        const bool m_scaleToScreen = false;
    };
}

namespace
{
    class GbmContext : public QEGLPlatformContext
    {
        using Inherited = QEGLPlatformContext;

      public:
        GbmContext( const QOpenGLContext* context, EGLDisplay display, EGLConfig config )
            : QEGLPlatformContext( context->format(), context->shareHandle(),
                display, &config, context->nativeHandle(), Flags() )
        {
        }

        EGLSurface eglSurfaceForPlatformSurface( QPlatformSurface* surface ) override
        {
            if( auto window = dynamic_cast< GbmWindow* >( surface ) )
                return window->eglSurface();

            if( auto offscreenSurface = dynamic_cast< GbmOffscreenSurface* >( surface ) )
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
    };
}

namespace
{
    class GbmScreen : public QPlatformScreen
    {
        using Inherited = QPlatformScreen;

      public:
        GbmScreen( const QString& name, const QSize& size )
            : m_name( name )
            , m_geometry( 0, 0, size.width(), size.height() )
        {
        }

        QString name() const override { return m_name; }
        QRect geometry() const override { return m_geometry; }
        int depth() const override { return 32; }
        QImage::Format format() const override { return QImage::Format_RGB32; }

        qreal refreshRate() const override { return Inherited::refreshRate(); }

      private:
        const QString m_name;
        const QRect m_geometry;
    };
}

namespace
{
    class GbmIntegration : public QPlatformIntegration
    {
      public:
        void initialize() override;
        void destroy() override;

        bool hasCapability( QPlatformIntegration::Capability ) const override;

        QPlatformWindow* createPlatformWindow( QWindow* ) const override;
        QPlatformOffscreenSurface* createPlatformOffscreenSurface( QOffscreenSurface* ) const;

        QPlatformBackingStore* createPlatformBackingStore( QWindow* ) const override;

        QPlatformOpenGLContext* createPlatformOpenGLContext( QOpenGLContext* ) const override;
        QAbstractEventDispatcher* createEventDispatcher() const override;

        QPlatformFontDatabase* fontDatabase() const override;
        QPlatformInputContext* inputContext() const override;

      private:
        mutable QGenericUnixFontDatabase m_fontDatabase;
        QPlatformInputContext* m_inputContext = nullptr;
    };

    void GbmIntegration::initialize()
    {
        m_inputContext = QPlatformInputContextFactory::create();

        auto manager = QInputDeviceManagerPrivate::get(
            QGuiApplicationPrivate::inputDeviceManager() );

        manager->setDeviceCount( QInputDeviceManager::DeviceTypePointer, 1 );
        manager->setDeviceCount( QInputDeviceManager::DeviceTypeKeyboard, 1 );

        auto screen = new GbmScreen( "offscreen", QSize( 2000, 2000 ) );
        QWindowSystemInterface::handleScreenAdded( screen, true );
    }

    void GbmIntegration::destroy()
    {
    }

    QPlatformOpenGLContext*
        GbmIntegration::createPlatformOpenGLContext( QOpenGLContext* context ) const
    {
        auto gbmContext = new GbmContext( context, QGbm::eglDisplay(), QGbm::eglConfig() );

        if ( context->nativeHandle().isNull() )
        {
            const QEGLNativeContext nativeContext( gbmContext, QGbm::eglDisplay() );
            context->setNativeHandle( QVariant::fromValue( nativeContext ) );
        }

        return gbmContext;
    }

    bool GbmIntegration::hasCapability( QPlatformIntegration::Capability cap ) const
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

    QPlatformBackingStore* GbmIntegration::createPlatformBackingStore( QWindow* ) const
    {
        return nullptr;
    }

    QPlatformWindow* GbmIntegration::createPlatformWindow( QWindow* window ) const
    {
        return new GbmWindow( window );
    }

    QPlatformOffscreenSurface* GbmIntegration::createPlatformOffscreenSurface(
        QOffscreenSurface* surface ) const
    {
        return new GbmOffscreenSurface( surface );
    }

    QAbstractEventDispatcher* GbmIntegration::createEventDispatcher() const
    {
        return createUnixEventDispatcher();
    }

    QPlatformFontDatabase* GbmIntegration::fontDatabase() const { return &m_fontDatabase; }
    QPlatformInputContext* GbmIntegration::inputContext() const { return m_inputContext; }
}

namespace
{
    class Plugin final : public QPlatformIntegrationPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA( IID QPlatformIntegrationFactoryInterface_iid FILE "metadata.json" )

      public:

        QPlatformIntegration* create( const QString& system, const QStringList& ) override
        {
            if ( system.compare( QStringLiteral( "gbm" ), Qt::CaseInsensitive ) == 0 )
            {
                return new GbmIntegration();
            }

            return nullptr;
        }
    };
}

#include "QGbmIntegrationPlugin.moc"

