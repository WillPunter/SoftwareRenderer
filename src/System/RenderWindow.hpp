/*  RenderWindow.hpp

    An abstract class that combines the functionality of a window (event
    handling, blitting a bitmap, etc.) and a render buffer (writing pixel data
    to the screen). */

#ifndef RENDER_WINDOW_HPP
#define RENDER_WINDOW_HPP

#include <cstdint>
#include <memory>

namespace System {

class RenderWindow {
    public:
        virtual bool handle_events() = 0;

        virtual void close_window() = 0;

        virtual bool is_open() = 0;

        virtual void clear_window() = 0;

        virtual void display_render_buffer() = 0;

        virtual void draw_pixel(int width, int height, uint8_t red,
            uint8_t green, uint8_t blue) = 0;
};

std::unique_ptr<RenderWindow> make_render_window(std::string title, int width,
    int height);

}

#endif