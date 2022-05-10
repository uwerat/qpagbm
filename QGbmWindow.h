#pragma once

#include <qpa/qplatformwindow.h>

class QGbmWindow final : public QPlatformWindow
{
    using Inherited = QPlatformWindow;

  public:
    QGbmWindow( QWindow* );

    void setGeometry( const QRect& ) override;
    void requestActivateWindow() override;

    qreal devicePixelRatio() const override;
};
