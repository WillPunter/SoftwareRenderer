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
#include <vector>

namespace System {

union pixel {
/*  Typical little endian RGBX structure. If in future, different formats are
    required (still expected to be 32 bits), then additional structures can be
    added to the union.
    
    Ideally any decision making in the rendering logic regarding the relative
    locations of R, G and B should be done at compile time. Keep this in mind
    for any future cross platform support. */
struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t x;
} rgb;

uint32_t data;
};

/*  Window abstract class declaration - note that currently, the winow assumes
    that the underlying platform supports true colour (24-bit colour depth) and
    is running on a little endian system (to ensure the RGB order of pixels in
    the buffer is consistent). Flexibility for other systems is expected to be
    added in future updates (see the README). */
class Window {
    public:
        virtual bool handle_events() = 0;
        virtual void clear_screen() = 0;        
        virtual void update_buffer() = 0;

        virtual void close_window() = 0;
        virtual bool is_closed() const = 0;

        virtual std::vector<pixel>& get_render_buffer()=0;
};

std::unique_ptr<Window> make_window(std::string title, int width, int height,
    int scaling);

}

#endif