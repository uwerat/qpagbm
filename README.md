# A Qt platform plugin for running Qt/OpenGL applications offscreen

- https://doc.qt.io/qt-6/qpa.html
- https://www.khronos.org/registry/EGL/extensions/MESA/EGL_MESA_platform_gbm.txt

It is supposed to work with all Qt versions >= 5.0

This platform allows postprocessing of the framebuffer without displying anything
on physical screens. The anticipated use cases are remote desktop solutions,

- remote desktop solutions ( f.e https://github.com/uwerat/vnc-eglfs )
- "screen" recorder
- test/development scenarios
- ...

My own motivation for this platform is related to the EGLFS/VNC server 
project ( https://github.com/uwerat/vnc-eglfs ).

# Qt/Quick

For a Qt/Quick window code might look like this

```
class Streamer : public QObject
{
    public:
        Streamer( QQuickWindow* window )
            : m_window( window )
        {
            connect( window, &QQuickWindow::frameSwapped,
                     this, &Streamer::grabWindow, Qt::DirectConnection );
        }

    private:
        void grabWindow()
        {
            // we are on the scene graph thread !

            extern QImage qt_gl_read_framebuffer(
                const QSize&, bool alpha_format, bool include_alpha );

            auto frameBuffer = qt_gl_read_framebuffer( m_window.size(), false, false );
            ...
        }

        QQuickWindow* m_window;
};

```
