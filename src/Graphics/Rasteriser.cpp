/*  Rasteriser.cpp 

    Implementation of rasterising functions. */

#include "Rasteriser.hpp"

#include <cmath>

namespace Graphics {

/*  Draw line with smallow gradient. */
inline static void draw_line_low(System::RenderWindow& window, pixel_coord p1,
    pixel_coord p2, uint8_t red, uint8_t green, uint8_t blue) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int y_inc = 1;

    if (dy < 0) {
        y_inc = -1;
        dy = -dy;
    }

    int diff = (2 * dy) - dx;
    int y = p1.y;

    for (int x = p1.x; x <= p2.x; x++) {
        draw_pixel(window, x, y, red, green, blue);

        if (diff > 0) {
            y += y_inc;
            diff += (2 * (dy - dx));
        } else {
            diff += 2 * dy;
        }
    }
}

inline static void draw_line_high(System::RenderWindow& window, pixel_coord p1,
    pixel_coord p2, uint8_t red, uint8_t green, uint8_t blue) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int x_inc = 1;

    if (dx < 0) {
        x_inc = -1;
        dx = -dx;
    }

    int diff = (2 * dx) - dy;
    int x = p1.x;

    for (int y = p1.y; y <= p2.y; y++) {
        draw_pixel(window, x, y, red, green, blue);

        if (diff > 0) {
            x += x_inc;
            diff += 2 * (dx - dy);
        } else {
            diff += 2 * dx;
        }
    }
}

void draw_pixel(System::RenderWindow& window, int x, int y, uint8_t red,
    uint8_t green, uint8_t blue) {
    window.draw_pixel(x, y, red, green, blue);
}

void draw_line(System::RenderWindow& window, pixel_coord p1, pixel_coord p2,
    uint8_t red, uint8_t green, uint8_t blue) {
    if (abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            draw_line_low(window, p2, p1, red, green, blue);
        } else {
            draw_line_low(window, p1, p2, red, green, blue);
        }
    } else {
        if (p1.y > p2.y) {
            draw_line_high(window, p2, p1, red, green, blue);
        } else {
            draw_line_high(window, p1, p2, red, green, blue);
        }
    }
}

void draw_wireframe_triangle(System::RenderWindow& window, pixel_coord p1,
    pixel_coord p2, pixel_coord p3, uint8_t red, uint8_t green, uint8_t blue) {
    draw_line(window, p1, p2, red, green, blue);
    draw_line(window, p2, p3, red, green, blue);
    draw_line(window, p3, p1, red, green, blue);  
};

}