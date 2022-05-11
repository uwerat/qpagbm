#pragma once

#include <qpa/qplatformintegration.h>
#include <memory>

class QGbmIntegration : public QPlatformIntegration
{
  public:
    QGbmIntegration();
    ~QGbmIntegration();

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
    class PrivateData;
    std::unique_ptr< PrivateData > m_data;
};
