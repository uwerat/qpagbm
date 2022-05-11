#pragma once

#include <memory>

class QVariant;
class QEGLPlatformContext;

namespace QGbm
{
    void* gbmDevice();
    void* eglDisplay();
    void* eglConfig();

    QVariant nativeContextHandle( const QEGLPlatformContext* );
}
