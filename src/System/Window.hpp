/*  Window.hpp

    Defines an abstract class presenting the window functionality supported by
    this engine.
    
    It also declares the make_window function which should be implemented by
    all implementations to make a window for the corresponding platofrm.
    
    The window functionalities include the provision of a render buffer and an
    event system. */

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <cstdint>
#include <memory>

namespace System {

class Window {
    public:
        virtual bool handle_events() = 0;

        virtual void clear_screen() = 0;
        virtual void draw_pixel(
            int x,
            int y,
            uint8_t red,
            uint8_t green,
            uint8_t blue
        ) = 0;
        virtual void draw_rectangle(
            int x0,
            int y0,
            int x1,
            int y2,
            uint8_t red,
            uint8_t green,
            uint8_t blue
        ) = 0;
        virtual void update_buffer() = 0;

        virtual void close_window() = 0;
        virtual bool is_closed() const = 0;
};

std::unique_ptr<Window> make_window(std::string title, int width, int height);

}

#endif