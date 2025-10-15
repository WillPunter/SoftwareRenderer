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
        
        virtual int get_width() = 0;

        virtual int get_height() = 0;
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