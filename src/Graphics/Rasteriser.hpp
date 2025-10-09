/*  Rasteriser.hpp

    This includes the rasterisation logic. Since the rasteriser does not need
    to store any state, it is just implemented as a set of functions in a
    namespace. Note that the render window is passed as a reference to each
    parameter, rather than stored as state to avoid potential situations where
    a window may deallocate the render buffer, but the rasteriser may still
    store a pointer or reference in it's own state.*/

#ifndef RASTERISER_HPP
#define RASTERISER_HPP

#include "./../System/RenderWindow.hpp"

namespace Graphics {

struct pixel_coord {
    int x;
    int y;
};

/*  Simple wrapper around window.draw_pixel member function. */
void draw_pixel(System::RenderWindow& window, int x, int y, uint8_t red,
    uint8_t green, uint8_t blue);

/*  Bresenham's algorithm - TODO: write explanation for this in code. */
void draw_line(System::RenderWindow& window, pixel_coord p1, pixel_coord p2,
    uint8_t red, uint8_t green, uint8_t blue);

void draw_wireframe_triangle(System::RenderWindow& window, pixel_coord p1,
    pixel_coord p2, pixel_coord p3, uint8_t red, uint8_t green, uint8_t blue);

}

#endif