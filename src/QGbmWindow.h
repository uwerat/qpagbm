#pragma once

#include <qpa/qplatformwindow.h>

class QGbmSurface;

class QGbmWindow : public QPlatformWindow
{
    using Inherited = QPlatformWindow;

  public:
    QGbmWindow( QWindow* );
    ~QGbmWindow() override;

    void setGeometry( const QRect& ) override;
    void requestActivateWindow() override;

    QSurfaceFormat format() const override;

    qreal devicePixelRatio() const override;

    void* eglSurface() const;

  private:
    QGbmSurface* m_surface = nullptr;
};
