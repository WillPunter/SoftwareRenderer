/*  Rasteriser.cpp 

    Implementation of rasterising functions. */

#include "Rasteriser.hpp"

#include <cmath>
#include <algorithm>

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

void draw_wireframe_triangle(
    System::RenderWindow& window,
    pixel_coord p1,
    pixel_coord p2,
    pixel_coord p3,
    uint8_t red,
    uint8_t green,
    uint8_t blue
) {
    draw_line(window, p1, p2, red, green, blue);
    draw_line(window, p2, p3, red, green, blue);
    draw_line(window, p3, p1, red, green, blue);  
}

/*  Draw shaded pixel row - precondition is that p1.x <= p2.x and that
    p1.y == p2.y.
    
    We assume the following properties of the pixel_coord structures, as
    this function is intended to be passed from draw_shaded_triangle:
        - depth is the inverse depth 1 / z.
        - intensity is divided by the original depth intensity / z.
        - red, green and blue are divided by the original depth as well: r / z,
          g / z, b / z.
        - tex_x and tex_y are divided by the original depth as well: tex_x / z,
          tex_y / z. */
void draw_shaded_row(
    System::RenderWindow& window,
    int y,
    pixel_coord p1,
    pixel_coord p2
) {
    /*  To draw a perspective-correct row of a triangle, we need to determine
        the following properties for each pixel:
            - Depth (for depth buffering purposes).
            - Intenity (for lighting calculations).
            - Colour (for plane coordinates).
            - Texture coordinates (TODO).
            
        To draw each of the pixels, we interpolate between p1 and p2, each with
        y ordinate y (the parameter). 
    
        Note that the inverse of the depth (z) of points on the line varies
        linearly with the x coordinates in pixel space (see the "Mathematics of
        the Rendering Pipeline" file in the documentation to see a derivation).
        
        Since the intensity, colour and texture vary linearly with the depth
        (z ordinate) in camera space, we would like to interpolate with respect
        to the Z ordinate. This is not possible of course, as we interpolate
        with respect to the x ordinate of the pixels. Since 1/z varies linearly
        with the pixel x ordinate, we want to express the relationship of these
        attributes with the z ordinate in camera space in terms of 1/z:
            I   = mz + c (for some m and c)
            I/z = m + c/z 

        So I/z varies linearly with 1/z, so therefore we can linearly
        interpolate for I/z and compute I/z / (1 / z) to obtain I. */
    
    /*  Number of interpolation steps - all other quantities must be divided
        linearly into this number of steps. This is the number of increments
        we do, not the number of pixels drawn (which will be this + 1 for
        the first one). */
    int num_steps = abs(p2.x - p1.x);

    /*  Inverse depth variation. */
    double inv_z_diff = p2.depth - p1.depth;
    double inv_z_step = inv_z_diff / num_steps;
    double inv_z = p1.depth;

    /*  Intensity / z variation - varies linearly with 1/z. */
    double i_div_z_diff = p2.intensity - p1.intensity;
    double i_div_z_step = i_div_z_diff / num_steps;
    double i_div_z = p1.intensity;

    /*  Colour variation. */
    double r_div_z_diff = p2.red - p1.red;
    double g_div_z_diff = p2.green - p1.green;
    double b_div_z_diff = p2.blue - p1.blue;

    double r_div_z_step = r_div_z_diff / num_steps;
    double g_div_z_step = g_div_z_diff / num_steps;
    double b_div_z_step = b_div_z_diff / num_steps;

    double r_div_z = p1.red;
    double g_div_z = p1.green;
    double b_div_z = p1.blue;

    /*  Texture coordinates. */
    double tex_x_div_z_diff = p2.tex_x - p1.tex_x;
    double tex_y_div_z_diff = p2.tex_y - p1.tex_y;

    double tex_x_div_z_step = tex_x_div_z_diff / num_steps;
    double tex_y_div_z_step = tex_y_div_z_diff / num_steps;

    double tex_x_div_z = p1.tex_x;
    double tex_y_div_z = p1.tex_y;

    double intensity = 0.0;
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double tex_x = 0.0;
    double tex_y = 0.0;

    for (int i = p1.x; i <= p2.x; i++) {
        /*  Compute intensity by taking (I/z) / (1/z). */
        intensity = i_div_z / inv_z;
        r = r_div_z / inv_z;
        g = g_div_z / inv_z;
        b = b_div_z / inv_z;
        tex_x = tex_x_div_z / inv_z;
        tex_y = tex_y_div_z / inv_z;

        /*  Mix colours. */

        /*  Check depth buffer and pixel coordinates. */

        /*  Draw pixel. */
        draw_pixel(
            window,
            i,
            y,
            static_cast<uint8_t>(r),
            static_cast<uint8_t>(g),
            static_cast<uint8_t>(b)
        );

        /*  Increment interpolation steps. */
        inv_z += inv_z_step;
        i_div_z += i_div_z_step;
        r_div_z += r_div_z_step;
        g_div_z += g_div_z_step;
        b_div_z += b_div_z_step;
        tex_x_div_z += tex_x_div_z_step;
        tex_y_div_z += tex_y_div_z_step;
    }
}

static double clamp(double val, double low, double high) {
    if (val < low) {
        return low;
    } else if(val > high) {
        return high;
    }

    return val;
}

/*  Precondition - the depths of all of the provided coordinates are non-zero.
    This is because clipping will have already removed all coordinates behind
    the viewing plane, which is significantly far away from the 0
    z-ordinate. */
void draw_shaded_triangle(
    System::RenderWindow& window,
    pixel_coord p1,
    pixel_coord p2,
    pixel_coord p3
) {
    /*  Order points by y - p1 should be the lowest point, p2 the middle and
        p3 the highest (numerically speaking, in pixel space). */
    if (p1.y > p2.y) {
        pixel_coord temp = p1;
        p1 = p2;
        p2 = temp;
    }

    if (p2.y > p3.y) {
        pixel_coord temp = p2;
        p2 = p3;
        p3 = temp;
    }

    if (p1.y > p2.y) {
        pixel_coord temp = p1;
        p1 = p2;
        p2 = temp;
    }

    /*  Determine number of steps. */
    int num_steps_1_2 = p2.y - p1.y;
    int num_steps_1_3 = p3.y - p1.y;
    int num_steps_2_3 = p3.y - p2.y;

    /*  Due to our sorting, p1 -> p3 is the tallest side. If it's height is 0,
        then there is nothing to draw. */
    if (num_steps_1_3 == 0) {
        return;
    }

    /*  Declare interpolation variables. */
    double x_diff_1_2 {};
    double x_step_1_2 {};
    double x_1_2 {};
    double x_diff_1_3 {};
    double x_step_1_3 {};
    double x_1_3 {};
    double x_diff_2_3 {};
    double x_step_2_3 {};
    double x_2_3 {};
    double inv_z_diff_1_2 {};
    double inv_z_step_1_2 {};
    double inv_z_1_2 {};
    double inv_z_diff_1_3 {};
    double inv_z_step_1_3 {};
    double inv_z_1_3 {};
    double inv_z_diff_2_3 {};
    double inv_z_step_2_3 {};
    double inv_z_2_3 {};
    double i_div_z_diff_1_2 {};
    double i_div_z_step_1_2 {};
    double i_div_z_1_2 {};
    double i_div_z_diff_1_3 {};
    double i_div_z_step_1_3 {};
    double i_div_z_1_3 {};
    double i_div_z_diff_2_3 {};
    double i_div_z_step_2_3 {};
    double i_div_z_2_3 {};
    double r_div_z_diff_1_2 {};
    double g_div_z_diff_1_2 {};
    double b_div_z_diff_1_2 {};
    double r_div_z_step_1_2 {};
    double g_div_z_step_1_2 {};
    double b_div_z_step_1_2 {};
    double r_div_z_1_2 {};
    double g_div_z_1_2 {};
    double b_div_z_1_2 {};
    double r_div_z_diff_1_3 {};
    double g_div_z_diff_1_3 {};
    double b_div_z_diff_1_3 {};
    double r_div_z_step_1_3 {};
    double g_div_z_step_1_3 {};
    double b_div_z_step_1_3 {};
    double r_div_z_1_3 {};
    double g_div_z_1_3 {};
    double b_div_z_1_3 {};
    double r_div_z_diff_2_3 {};
    double g_div_z_diff_2_3 {};
    double b_div_z_diff_2_3 {};
    double r_div_z_step_2_3 {};
    double g_div_z_step_2_3 {};
    double b_div_z_step_2_3 {};
    double r_div_z_2_3 {};
    double g_div_z_2_3 {};
    double b_div_z_2_3 {};
    double tex_x_div_z_diff_1_2 {};
    double tex_y_div_z_diff_1_2 {};
    double tex_x_div_z_step_1_2 {};
    double tex_y_div_z_step_1_2 {};
    double tex_x_div_z_1_2 {};
    double tex_y_div_z_1_2 {};
    double tex_x_div_z_diff_1_3 {};
    double tex_y_div_z_diff_1_3 {};
    double tex_x_div_z_step_1_3 {};
    double tex_y_div_z_step_1_3 {};
    double tex_x_div_z_1_3 {};
    double tex_y_div_z_1_3 {};
    double tex_x_div_z_diff_2_3 {};
    double tex_y_div_z_diff_2_3 {};
    double tex_x_div_z_step_2_3 {};
    double tex_y_div_z_step_2_3 {};
    double tex_x_div_z_2_3 {};
    double tex_y_div_z_2_3 {};

    /*  We have already verified that the p1 -> p3 line has some height,
        therefore we can compute the interpolation values without fear of a
        divide by zero error. */
    x_diff_1_3 = p3.x - p1.x;
    x_step_1_3 = x_diff_1_3 / num_steps_1_3;
    x_1_3 = p1.x;

    inv_z_diff_1_3 = (1.0 / p3.depth) - (1.0 / p1.depth);
    inv_z_step_1_3 = inv_z_diff_1_3 / num_steps_1_3;
    inv_z_1_3 = 1.0 / p1.depth;

    i_div_z_diff_1_3 = (p3.intensity / p3.depth) - (p1.intensity / p1.depth);
    i_div_z_step_1_3 = i_div_z_diff_1_3 / num_steps_1_3;
    i_div_z_1_3 = p1.intensity / p1.depth;

    r_div_z_diff_1_3 = (p3.red / p3.depth) - (p1.red / p1.depth);
    g_div_z_diff_1_3 = (p3.green / p3.depth) - (p1.green / p1.depth);
    b_div_z_diff_1_3 = (p3.blue / p3.depth) - (p1.blue / p1.depth);

    r_div_z_step_1_3 = r_div_z_diff_1_3 / num_steps_1_3;
    g_div_z_step_1_3 = g_div_z_diff_1_3 / num_steps_1_3;
    b_div_z_step_1_3 = b_div_z_diff_1_3 / num_steps_1_3;

    r_div_z_1_3 = p1.red / p1.depth;
    g_div_z_1_3 = p1.green / p1.depth;
    b_div_z_1_3 = p1.blue / p1.depth;

    tex_x_div_z_diff_1_3 = (p3.tex_x / p3.depth) - (p1.tex_x / p1.depth);
    tex_y_div_z_diff_1_3 = (p3.tex_y / p3.depth) - (p1.tex_y / p1.depth);

    tex_x_div_z_step_1_3 = tex_x_div_z_diff_1_3 / num_steps_1_3;
    tex_y_div_z_step_1_3 = tex_y_div_z_diff_1_3 / num_steps_1_3;

    tex_x_div_z_1_3 = p1.tex_x / p1.depth;
    tex_y_div_z_1_3 = p1.tex_y / p1.depth;

    /*  If the lower triangle exists (i.e. it's height is nonzero) then we can
        compute it's coordinates and render it. */
    if (num_steps_1_2 > 0) {
        x_diff_1_2 = p2.x - p1.x;
        x_step_1_2 = x_diff_1_2 / num_steps_1_2;
        x_1_2 = p1.x;

        inv_z_diff_1_2 = (1.0 / p2.depth) - (1.0 / p1.depth);
        inv_z_step_1_2 = inv_z_diff_1_2 / num_steps_1_2;
        inv_z_1_2 = 1.0 / p1.depth;

        i_div_z_diff_1_2 = (p2.intensity / p2.depth) - (p1.intensity / p1.depth);
        i_div_z_step_1_2 = i_div_z_diff_1_2 / num_steps_1_2;
        i_div_z_1_2 = p1.intensity / p1.depth;

        r_div_z_diff_1_2 = (p2.red / p2.depth) - (p1.red / p1.depth);
        g_div_z_diff_1_2 = (p2.green / p2.depth) - (p1.green / p1.depth);
        b_div_z_diff_1_2 = (p2.blue / p2.depth) - (p1.blue / p1.depth);

        r_div_z_step_1_2 = r_div_z_diff_1_2 / num_steps_1_2;
        g_div_z_step_1_2 = g_div_z_diff_1_2 / num_steps_1_2;
        b_div_z_step_1_2 = b_div_z_diff_1_2 / num_steps_1_2;

        r_div_z_1_2 = p1.red / p1.depth;
        g_div_z_1_2 = p1.green / p1.depth;
        b_div_z_1_2 = p1.blue / p1.depth;

        tex_x_div_z_diff_1_2 = (p2.tex_x / p2.depth) - (p1.tex_x / p1.depth);
        tex_y_div_z_diff_1_2 = (p2.tex_y / p2.depth) - (p1.tex_y / p1.depth);

        tex_x_div_z_step_1_2 = tex_x_div_z_diff_1_2 / num_steps_1_2;
        tex_y_div_z_step_1_2 = tex_y_div_z_diff_1_2 / num_steps_1_2;

        tex_x_div_z_1_2 = p1.tex_x / p1.depth;
        tex_y_div_z_1_2 = p1.tex_y / p1.depth;

        /*  Draw lower triangle. */
        for (int i = p1.y; i <= p2.y; i++) {
            /*  Draw row. */
            pixel_coord p_1_2 = {
                (int) x_1_2,
                i,
                inv_z_1_2,
                i_div_z_1_2,
                r_div_z_1_2,
                g_div_z_1_2,
                b_div_z_1_2,
                tex_x_div_z_1_2,
                tex_y_div_z_1_2
            };

            pixel_coord p_1_3 = {
                (int) x_1_3,
                i,
                inv_z_1_3,
                i_div_z_1_3,
                r_div_z_1_3,
                g_div_z_1_3,
                b_div_z_1_3,
                tex_x_div_z_1_3,
                tex_y_div_z_1_3
            };

            if (x_1_2 <= x_1_3) {
                draw_shaded_row(
                    window,
                    i,
                    p_1_2,
                    p_1_3
                );
            } else {
                draw_shaded_row(
                    window,
                    i,
                    p_1_3,
                    p_1_2
                );
            }

            /*  Increment interpolation steps for attributes. */
            x_1_2 += x_step_1_2;
            x_1_3 += x_step_1_3;

            inv_z_1_2 += inv_z_step_1_2;
            inv_z_1_3 += inv_z_step_1_3;

            i_div_z_1_2 += i_div_z_step_1_2;
            i_div_z_1_3 += i_div_z_step_1_3;

            r_div_z_1_2 += r_div_z_step_1_2;
            r_div_z_1_3 += r_div_z_step_1_3;

            g_div_z_1_2 += g_div_z_step_1_2;
            g_div_z_1_3 += g_div_z_step_1_3;

            b_div_z_1_2 += b_div_z_step_1_2;
            b_div_z_1_3 += b_div_z_step_1_3;

            tex_x_div_z_1_2 += tex_x_div_z_step_1_2;
            tex_x_div_z_1_3 += tex_x_div_z_step_1_3;

            tex_y_div_z_1_2 += tex_y_div_z_step_1_2;
            tex_y_div_z_1_3 += tex_y_div_z_step_1_3;
        }
    }

    /*  If the upper triangle exists (i.e. it's height is nonzer) then we can
        compute it's coordinates and render it. */
    if (num_steps_2_3 > 0) {
        /*  X interpolation. */
        x_diff_2_3 = p3.x - p2.x;
        x_step_2_3 = x_diff_2_3 / num_steps_2_3;
        x_2_3 = p2.x;

        /*  Inverse depth interpolation. */
        inv_z_diff_2_3 = (1.0 / p3.depth) - (1.0 / p2.depth);
        inv_z_step_2_3 = inv_z_diff_2_3 / num_steps_2_3;
        inv_z_2_3 = 1.0 / p2.depth;

        /*  Intensity / depth interpolation. */

        i_div_z_diff_2_3 = (p3.intensity / p3.depth) - (p2.intensity / p2.depth);
        i_div_z_step_2_3 = i_div_z_diff_2_3 / num_steps_2_3;
        i_div_z_2_3 = p2.intensity / p2.depth;

        /*  Colour interpolation. */

        r_div_z_diff_2_3 = (p3.red / p3.depth) - (p2.red / p2.depth);
        g_div_z_diff_2_3 = (p3.green / p3.depth) - (p2.green / p2.depth);
        b_div_z_diff_2_3 = (p3.blue / p3.depth) - (p2.blue / p2.depth);

        r_div_z_step_2_3 = r_div_z_diff_2_3 / num_steps_2_3;
        g_div_z_step_2_3 = g_div_z_diff_2_3 / num_steps_2_3;
        b_div_z_step_2_3 = b_div_z_diff_2_3 / num_steps_2_3;

        r_div_z_2_3 = p2.red / p2.depth;
        g_div_z_2_3 = p2.green / p2.depth;
        b_div_z_2_3 = p2.blue / p2.depth;

        /*  Texture coordinates. */
        tex_x_div_z_diff_2_3 = (p3.tex_x / p3.depth) - (p2.tex_x / p2.depth);
        tex_y_div_z_diff_2_3 = (p3.tex_y / p3.depth) - (p2.tex_y / p2.depth);

        tex_x_div_z_step_2_3 = tex_x_div_z_diff_2_3 / num_steps_2_3;
        tex_y_div_z_step_2_3 = tex_y_div_z_diff_2_3 / num_steps_2_3;

        tex_x_div_z_2_3 = p2.tex_x / p2.depth;
        tex_y_div_z_2_3 = p2.tex_y / p2.depth;

        /*  Draw upper triangle. */
        for (int i = p2.y; i <= p3.y; i++) {
            /*  Draw row. */
            pixel_coord p_2_3 = {
                (int) x_2_3,
                i,
                inv_z_2_3,
                i_div_z_2_3,
                r_div_z_2_3,
                g_div_z_2_3,
                b_div_z_2_3,
                tex_x_div_z_2_3,
                tex_y_div_z_2_3
            };

            pixel_coord p_1_3 = {
                (int) x_1_3,
                i,
                inv_z_1_3,
                i_div_z_1_3,
                r_div_z_1_3,
                g_div_z_1_3,
                b_div_z_1_3,
                tex_x_div_z_1_3,
                tex_y_div_z_1_3
            };

            if (x_2_3 <= x_1_3) {
                draw_shaded_row(
                    window,
                    i,
                    p_2_3,
                    p_1_3
                );
            } else {
                draw_shaded_row(
                    window,
                    i,
                    p_1_3,
                    p_2_3
                );
            }

            /*  Increment interpolation steps for attributes. */
            x_2_3 += x_step_2_3;
            x_1_3 += x_step_1_3;

            inv_z_2_3 += inv_z_step_2_3;
            inv_z_1_3 += inv_z_step_1_3;

            i_div_z_2_3 += i_div_z_step_2_3;
            i_div_z_1_3 += i_div_z_step_1_3;

            r_div_z_2_3 += r_div_z_step_2_3;
            r_div_z_1_3 += r_div_z_step_1_3;

            g_div_z_2_3 += g_div_z_step_2_3;
            g_div_z_1_3 += g_div_z_step_1_3;

            b_div_z_2_3 += b_div_z_step_2_3;
            b_div_z_1_3 += b_div_z_step_1_3;

            tex_x_div_z_2_3 += tex_x_div_z_step_2_3;
            tex_x_div_z_1_3 += tex_x_div_z_step_1_3;

            tex_y_div_z_2_3 += tex_y_div_z_step_2_3;
            tex_y_div_z_1_3 += tex_y_div_z_step_1_3;
        }
    }
}

}