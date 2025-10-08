/*  X11Window.hpp */

#ifndef X11WINDOW_HPP
#define X11WINDOW_HPP

#include <X11/Xlib.h>
#include <string>

namespace System {

/*  Forward declare implementing classes so that they can be made friends. */
class X11RGBARenderWindow;

class X11Window {
    public:
        /*  Can only be constructed by X11 render window classes. There may be
            multiple of these to support performant blitting / rasterising on
            different hardware setups (e.g. TrueColor, Greyscale, etc.). */
        X11Window()                         = delete;

        /*  A window is not copyable or movable, but it is destroyable. To
            avoid rule-of-5 issues, we delete all rule-of-5 functions except
            for the destructor. */
        X11Window(X11Window &)              = delete;
        X11Window(X11Window &&)             = delete;
        X11Window& operator=(X11Window &)   = delete;
        X11Window&& operator=(X11Window &&) = delete;

        ~X11Window();

        bool handle_events();

        bool is_open();

        void close_window();

        friend X11RGBARenderWindow;

    private:
        X11Window(std::string title, int width, int height);

        bool multiplex_event(XEvent&);

        bool should_close = false;

        int width;
        int height;

        Display* server_connection;
        int screen_id;
        Window window;
        Visual* visual_info;

        Atom window_manager_delete_window_id;
};

}

#endif