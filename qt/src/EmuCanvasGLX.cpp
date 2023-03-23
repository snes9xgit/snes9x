#include "EmuCanvas.hpp"

EmuCanvas::EmuCanvas()
{
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

EmuCanvas::~EmuCanvas()
{
}

struct drawing_area : QWidget
{
  public:

    Window xid = 0;
    bool ready = false;
    XSetWindowAttributes xattr;
    Visual *visual;
    unsigned int xflags = 0;
    QWindow *wrapper_window = nullptr;
    QWidget *wrapper = nullptr;

    drawing_area()
    {
    }

    ~drawing_area()
    {
    }

    void recreateWindow()
    {
        if (xid)
        {
            XUnmapWindow(QX11Info::display(), xid);
            XDestroyWindow(QX11Info::display(), xid);
            xid = 0;
        }
        

        XSetErrorHandler([](Display *dpy, XErrorEvent *event) -> int{
            char text[4096];
            XGetErrorText(QX11Info::display(), event->error_code, text, 4096);
            printf("%s\n", text);
            return 0;
        });

        createWinId();

        int xwidth = width() * devicePixelRatio();
        int xheight = height() * devicePixelRatio();

        printf ("%d %d to %d %d %f\n", width(), height(), xwidth, xheight, devicePixelRatioFScale());

        memset(&xattr, 0, sizeof(XSetWindowAttributes));
        xattr.background_pixel = 0;
        xattr.backing_store = 0;
        xattr.event_mask = ExposureMask;
        xattr.border_pixel = 0;
        xflags = CWWidth | CWHeight | CWEventMask | CWBackPixel | CWBorderPixel | CWBackingStore;
        xid = XCreateWindow(QX11Info::display(), winId(), 0, 0, xwidth, xheight, 0, CopyFromParent, InputOutput, CopyFromParent, xflags, &xattr);
        XMapWindow(QX11Info::display(), xid);
        /*wrapper_window = QWindow::fromWinId((WId)xid);
        wrapper = QWidget::createWindowContainer(wrapper_window, this); */
    }

    void paintEvent(QPaintEvent *event) override
    {
        

        return;

        auto id = winId();
        
        XGCValues gcvalues {};
        gcvalues.background = 0x00ff0000;

        gcvalues.foreground = 0x00ff0000;

        
        /*

        QPainter paint(this);
        QImage image((const uchar *)snes9x->GetScreen(), 256, 224, 1024, QImage::Format_RGB16);
        paint.drawImage(0, 0, image, 0, 0, 256, 224);
        paint.drawImage(QRect(0, 0, width(), height()), image, QRect(0, 0, 256, 224));
        paint.end();
        ready = false; */
    }

    void draw()
    {
        ready = true;
        update();
    }

    void resizeEvent(QResizeEvent *event) override
    {
        recreateWindow();
    }
};
