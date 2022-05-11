#pragma once

#include <qpa/qplatformoffscreensurface.h>

class QGbmSurface;

class QGbmOffscreenSurface : public QPlatformOffscreenSurface
{
  public:
    QGbmOffscreenSurface( QOffscreenSurface* );
    ~QGbmOffscreenSurface() override;

    void* eglSurface() const;

    QSurfaceFormat format() const override;
    bool isValid() const override;

  private:
    QGbmSurface* m_surface = nullptr;
};
