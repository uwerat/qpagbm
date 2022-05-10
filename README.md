# A Qt platform plugin for running Qt/OpenGL applications offscreen

- https://doc.qt.io/qt-6/qpa.html
- https://www.khronos.org/registry/EGL/extensions/MESA/EGL_MESA_platform_gbm.txt

It is supposed to work with all Qt versions >= 5.0

This platform allows running OpenGL applications without displaying anything
on physical screens. The anticipated use cases will postprocess the frames:

- remote desktop solutions ( f.e https://github.com/uwerat/vnc-eglfs )
- "screen" recorder
- test/development scenarios
- ...

My own motivation for this platform plugin is related to the EGLFS/VNC server 
project ( https://github.com/uwerat/vnc-eglfs ).

# Qt/Quick

For a Qt/Quick code might look like this

```
class FrameHandler : public QObject
{
    public:
        FrameHandler( QQuickWindow* window )
            : m_window( window )
        {
            connect( window, &QQuickWindow::frameSwapped,
                     this, &FrameHandler::grabWindow, Qt::DirectConnection );
        }

    private:
        void grabWindow()
        {
            /*
                Still on the scene graph thread, ready to postprocess the frame.
                Asn an example the code below grabs it into an image.
             */

            extern QImage qt_gl_read_framebuffer(
                const QSize&, bool alpha_format, bool include_alpha );

            auto frameBuffer = qt_gl_read_framebuffer( m_window.size(), false, false );
            ...
        }

        QQuickWindow* m_window;
};

```
