/*  RenderWindow.hpp

    An abstract class that combines the functionality of a window (event
    handling, blitting a bitmap, etc.) and a render buffer (writing pixel data
    to the screen). */

#ifndef RENDER_WINDOW_HPP
#define RENDER_WINDOW_HPP

#include <cstdint>
#include <memory>

namespace System {

/*  Different platforms encode the same keys with different numbers. Hence, we
    provide a platform independent set of key encodings that can be used by
    applications using the library. */
enum class KeySymbol {
    SPACE = 32,

    ARROW_LEFT  = 37,
    ARROW_UP    = 38,
    ARROW_RIGHT = 39,
    ARROW_DOWN  = 40,

    A = 65,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z
};

enum class KeyState {
    KEY_UP,
    KEY_DOWN,
    KEY_UNDEFINED
};

class RenderWindow {
    public:
        virtual bool handle_events() = 0;

        virtual void close_window() = 0;

        virtual bool is_open() = 0;

        virtual void clear_window() = 0;

        virtual void display_render_buffer() = 0;

        virtual void draw_pixel(int width, int height, uint8_t red,
            uint8_t green, uint8_t blue) = 0;
        
        virtual int get_width() = 0;

        virtual int get_height() = 0;

        virtual KeyState get_key(KeySymbol key_id) = 0;
};

/*  RenderWindow factory method. This constructs some instance of one of the
    RenderWindow derived classes under the RenderWindow apparent type. The idea
    is that different platforms can return different subclasses for their own
    implementation.
    
    NOTE: it is expected that this allocates a new RenderWindow subclass object
    on the heap (i.e. using new). This is for flexibility purposes, although
    for most, but not necessarily all, use-cases it is likely that we will want
    to wrap it in a smart pointer to avoid forgetting to deallocate it. */
RenderWindow* make_render_window(std::string title, int width, int height);

}

#endif