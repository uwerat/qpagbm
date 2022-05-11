#pragma once

#include <qsurfaceformat.h>
#include <qdatastream.h>

class QGbmSurface
{
  public:
    QGbmSurface( const QSize& );
    ~QGbmSurface();

    void* eglSurface() const;
    QSurfaceFormat format() const;

  private:
    void* m_gbmSurface = nullptr;
    void* m_eglSurface = nullptr;

    QSurfaceFormat m_format;
};

inline void* QGbmSurface::eglSurface() const
{
    return m_eglSurface;
}

inline QSurfaceFormat QGbmSurface::format() const
{
    return m_format;;
}
