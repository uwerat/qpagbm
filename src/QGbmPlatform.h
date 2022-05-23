#pragma once

#include <qsurfaceformat.h>

class QEGLPlatformContext;

namespace QGbm
{
    /*
        egl/gbm and Qt header have conflicts. So we better work with
        void* handles instead
     */
    void* eglDisplay();
    void* eglConfig();

    QSurfaceFormat surfaceFormat();

    void* createEglSurface( void* gbmSurface );
    void* createGbmSurface( int width, int height );

    void destroySurfaces( void* gbmSurface, void* eglSurface );
}
