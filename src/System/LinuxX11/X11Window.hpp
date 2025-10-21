/*  X11Window.hpp */

#ifndef X11WINDOW_HPP
#define X11WINDOW_HPP

#include <X11/Xlib.h>
#include <string>
#include "./../RenderWindow.hpp"

constexpr int KEY_COUNT = 256;

namespace System {

/*  X11 representation of arrow keys. Unfortunately this cannot be an enum
    class since it must be compared with X11 integer values returned by it's C
    API functions. */
enum X11_ARROW_KEYS {
    X11_ARROW_LEFT = 65361,
    X11_ARROW_UP,
    X11_ARROW_RIGHT,
    X11_ARROW_DOWN
};

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

        KeyState get_key(KeySymbol key_id);

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

        /*  Separate ascii and non-ascii keys due to representation differences
            in X11 - ascii keys (characters, numbers, punctuation) have their
            expected numerical values, but control keys - enter, arrows, etc.
            have unique numerical encodings. */
        KeyState ascii_keys[KEY_COUNT];
        KeyState arrow_keys[4];
};

}

#endif