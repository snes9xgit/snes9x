#include <gtkmm.h>
#include <gdk/gdkx.h>
#include "vk2d.hpp"

int main(int argc, char *argv[])
{
    XInitThreads();
    auto application = Gtk::Application::create(argc, argv, "org.bearoso.vulkantest");

    Gtk::Window window;
    Gtk::Button button;
    Gtk::DrawingArea drawingarea;
    Gtk::VBox vbox;

    window.set_title("Vulkan Test");
    window.set_events(Gdk::EventMask::ALL_EVENTS_MASK);
    button.set_label("Close");

    vbox.pack_start(drawingarea, true, true, 0);
    vbox.pack_start(button, false, false, 0);
    vbox.set_spacing(5);
    button.set_hexpand(false);
    button.set_halign(Gtk::ALIGN_END);

    window.add(vbox);
    window.set_border_width(5);
    vbox.show_all();

    button.signal_clicked().connect([&] { 
        window.close(); 
    }); 

    window.resize(640, 480);
    window.show_all();

    Window xid = gdk_x11_window_get_xid(drawingarea.get_window()->gobj());
    Display *dpy = gdk_x11_display_get_xdisplay(drawingarea.get_display()->gobj());

    vk2d vk2d;

    vk2d.init_xlib_instance();
    vk2d.attach(dpy, xid);
    vk2d.init_device();

    drawingarea.signal_configure_event().connect([&](GdkEventConfigure *event) {
        vk2d.recreate_swapchain();
        return false;
    });

    window.signal_key_press_event().connect([&](GdkEventKey *key) -> bool {
        printf ("Key press %d\n", key->keyval);
        return false;
    }, false);

    window.signal_key_release_event().connect([&](GdkEventKey *key) -> bool {
        printf ("Key release %d\n", key->keyval);
        return false;
    }, false);

    drawingarea.set_app_paintable(true);

    drawingarea.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context> &context) -> bool {
        return true;
    });

    auto id = Glib::signal_idle().connect([&]{
        vk2d.draw();
        vk2d.wait_idle();
        return true;
    });

    window.signal_delete_event().connect([&](GdkEventAny *event) {
        id.disconnect();
        return false;
    });

    application->run(window);

    return 0;
}