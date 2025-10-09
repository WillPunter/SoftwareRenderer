/*  Rasteriser.hpp

    This includes the rasterisation logic. Since the rasteriser does not need
    to store any state, it is just implemented as a set of functions in a
    namespace. Note that the render window is passed as a reference to each
    parameter, rather than stored as state to avoid potential situations where
    a window may deallocate the render buffer, but the rasteriser may still
    store a pointer or reference in it's own state.*/

#ifndef RASTERISER_HPP
#define RASTERISER_HPP

namespace Graphics {

void draw_pixel(RenderWindow& window, int x, int y, uint8_t red,
    uint8_t green, uint8_t blue);
    window.
}

}

#endif