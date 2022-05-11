#include "QGbmSurface.h"
#include "QGbmPlatform.h"

#include <qdebug.h>
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

    EGLSurface createEglWindowSurface( EGLDisplay eglDisplay,
        EGLConfig eglConfig, void* gbmSurface )
    {
        const auto native = reinterpret_cast< EGLNativeWindowType >( gbmSurface );
        return eglCreateWindowSurface( eglDisplay, eglConfig, native, nullptr );
    }
}

QGbmSurface::QGbmSurface( const QSize& size )
{
    auto gbmDevice = static_cast< gbm_device* >( QGbm::gbmDevice() );
    auto eglConfig = QGbm::eglConfig();
    auto eglDisplay = QGbm::eglDisplay();

    m_gbmSurface = gbm_surface_create( gbmDevice, size.width(), size.height(),
        GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING );

    m_eglSurface = createEglWindowSurface(
        eglDisplay, eglConfig, m_gbmSurface );

    Q_ASSERT( m_eglSurface != EGL_NO_SURFACE );
    if ( m_eglSurface == EGL_NO_SURFACE )
        qDebug() << "eglCreateWindowSurface:" << getEglErrorString();

    if ( m_eglSurface != EGL_NO_SURFACE )
        m_format = q_glFormatFromConfig( eglDisplay, eglConfig );
}

QGbmSurface::~QGbmSurface()
{
    eglDestroySurface( QGbm::eglDisplay(), m_eglSurface );
    gbm_surface_destroy( reinterpret_cast< gbm_surface* >( m_gbmSurface ) );
}
