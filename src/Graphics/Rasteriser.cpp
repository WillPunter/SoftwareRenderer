/*  Rasteriser.cpp 

    Implementation of rasterising functions. */

#include "Rasteriser.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <string>

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

static double clamp(double val, double low, double high) {
    if (val < low) {
        return low;
    } else if(val > high) {
        return high;
    }

    return val;
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
    pixel_coord p2,
    Resources::TrueColourBitmap* bitmap_ptr,
    int buffer_width,
    int buffer_height
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
    double inv_z_diff = p2.inv_z - p1.inv_z;
    double inv_z_step = inv_z_diff / num_steps;
    double inv_z = p1.inv_z;

    /*  Intensity / z variation - varies linearly with 1/z. */
    double i_div_z_diff = p2.i_div_z - p1.i_div_z;
    double i_div_z_step = i_div_z_diff / num_steps;
    double i_div_z = p1.i_div_z;

    /*  Colour variation. */
    double r_div_z_diff = p2.r_div_z - p1.r_div_z;
    double g_div_z_diff = p2.g_div_z - p1.g_div_z;
    double b_div_z_diff = p2.b_div_z - p1.b_div_z;

    double r_div_z_step = r_div_z_diff / num_steps;
    double g_div_z_step = g_div_z_diff / num_steps;
    double b_div_z_step = b_div_z_diff / num_steps;

    double r_div_z = p1.r_div_z;
    double g_div_z = p1.g_div_z;
    double b_div_z = p1.b_div_z;

    /*  Texture coordinates. */
    double tex_x_div_z_diff = p2.tex_x_div_z - p1.tex_x_div_z;
    double tex_y_div_z_diff = p2.tex_y_div_z - p1.tex_y_div_z;

    double tex_x_div_z_step = tex_x_div_z_diff / num_steps;
    double tex_y_div_z_step = tex_y_div_z_diff / num_steps;

    double tex_x_div_z = p1.tex_x_div_z;
    double tex_y_div_z = p1.tex_y_div_z;

    double intensity = 0.0;
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double tex_x = 0.0;
    double tex_y = 0.0;

    int p1_x = (int) floor(p1.x);
    int p2_x = (int) floor(p2.x);

    for (int i = p1_x; i <= p2_x; i++) {
        /*  Compute intensity by taking (I/z) / (1/z). */
        intensity = i_div_z / inv_z;
        r = r_div_z / inv_z;
        g = g_div_z / inv_z;
        b = b_div_z / inv_z;
        tex_x = tex_x_div_z / inv_z;
        tex_y = tex_y_div_z / inv_z;

        /*  Check depth buffer and pixel coordinates. */
        double depth_buff_val = window.read_depth_buffer(i, y);
        // std::cout << "Depth mine = " << std::to_string(inv_z) << " other = " << std::to_string(depth_buff_val) << std::endl;
        //std::cout << "i = " << std::to_string(i) << ", y = " << std::to_string(y) << ", buffer w,h = " << std::to_string(buffer_width) << ", " << std::to_string(buffer_height) << std::endl;
        if (inv_z > depth_buff_val && i >= 0 && i < buffer_width && y >= 0 && y <= buffer_height) {
            //std::cout << "display" << std::endl;
            double mix_r = r;
            double mix_g = g;
            double mix_b = b;

            /*  Check if textures are used. */
            if (bitmap_ptr != nullptr) {
                /*  Mix texture colours and rgb values. */
                int pixel_x = round(tex_x * (bitmap_ptr->width - 1));
                int pixel_y = round(tex_y * (bitmap_ptr->height - 1));

                if (pixel_x < 0) {
                    pixel_x = 0;
                } else if (pixel_x > bitmap_ptr->width - 1) {
                    pixel_x = bitmap_ptr->width - 1;
                }

                if (pixel_y < 0) {
                    pixel_y = 0;
                } else if (pixel_y > bitmap_ptr->height - 1) {
                    pixel_y = bitmap_ptr->height - 1;
                }

                pixel_y = bitmap_ptr->height - 1 - pixel_y;

                // std::cout << "Pix " << std::to_string(pixel_x) << " " << std::to_string(pixel_y) << std::endl;

                int pixel_index = pixel_y * bitmap_ptr->width + pixel_x;

                mix_r = bitmap_ptr->pixels[pixel_index].r * (mix_r / 255.0);
                mix_g = bitmap_ptr->pixels[pixel_index].g * (mix_g / 255.0);
                mix_b = bitmap_ptr->pixels[pixel_index].b * (mix_b / 255.0);
            }

            /*  Draw pixel. */
            draw_pixel(
                window,
                i,
                y,
                static_cast<uint8_t>(clamp(mix_r * intensity, 0, 255)),
                static_cast<uint8_t>(clamp(mix_g * intensity, 0, 255)),
                static_cast<uint8_t>(clamp(mix_b * intensity, 0, 255))
            );

            window.write_depth_buffer(i, y, inv_z);
        }

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

/*  Precondition - the depths of all of the provided coordinates are non-zero.
    This is because clipping will have already removed all coordinates behind
    the viewing plane, which is significantly far away from the 0
    z-ordinate. */
void draw_shaded_triangle(
    System::RenderWindow& window,
    pixel_coord p1,
    pixel_coord p2,
    pixel_coord p3,
    Resources::TrueColourBitmap* bitmap_ptr,
    int buffer_width,
    int buffer_height
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
    int num_steps_1_2 = abs(p2.y - p1.y);
    int num_steps_1_3 = abs(p3.y - p1.y);
    int num_steps_2_3 = abs(p3.y - p2.y);

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

    inv_z_diff_1_3 = p3.inv_z - p1.inv_z;
    inv_z_step_1_3 = inv_z_diff_1_3 / num_steps_1_3;
    inv_z_1_3 = p1.inv_z;

    i_div_z_diff_1_3 = p3.i_div_z - p1.i_div_z;
    i_div_z_step_1_3 = i_div_z_diff_1_3 / num_steps_1_3;
    i_div_z_1_3 = p1.i_div_z;

    r_div_z_diff_1_3 = p3.r_div_z - p1.r_div_z;
    g_div_z_diff_1_3 = p3.g_div_z - p1.g_div_z;
    b_div_z_diff_1_3 = p3.b_div_z - p1.b_div_z;

    r_div_z_step_1_3 = r_div_z_diff_1_3 / num_steps_1_3;
    g_div_z_step_1_3 = g_div_z_diff_1_3 / num_steps_1_3;
    b_div_z_step_1_3 = b_div_z_diff_1_3 / num_steps_1_3;

    r_div_z_1_3 = p1.r_div_z;
    g_div_z_1_3 = p1.g_div_z;
    b_div_z_1_3 = p1.b_div_z;

    tex_x_div_z_diff_1_3 = p3.tex_x_div_z - p1.tex_x_div_z;
    tex_y_div_z_diff_1_3 = p3.tex_y_div_z - p1.tex_y_div_z;

    tex_x_div_z_step_1_3 = tex_x_div_z_diff_1_3 / num_steps_1_3;
    tex_y_div_z_step_1_3 = tex_y_div_z_diff_1_3 / num_steps_1_3;

    tex_x_div_z_1_3 = p1.tex_x_div_z;
    tex_y_div_z_1_3 = p1.tex_y_div_z;

    /*  If the lower triangle exists (i.e. it's height is nonzero) then we can
        compute it's coordinates and render it. */
    if (num_steps_1_2 > 0) {
        x_diff_1_2 = p2.x - p1.x;
        x_step_1_2 = x_diff_1_2 / num_steps_1_2;
        x_1_2 = p1.x;

        inv_z_diff_1_2 = p2.inv_z - p1.inv_z;
        inv_z_step_1_2 = inv_z_diff_1_2 / num_steps_1_2;
        inv_z_1_2 = p1.inv_z;

        i_div_z_diff_1_2 = p2.i_div_z - p1.i_div_z;
        i_div_z_step_1_2 = i_div_z_diff_1_2 / num_steps_1_2;
        i_div_z_1_2 = p1.i_div_z;

        r_div_z_diff_1_2 = p2.r_div_z - p1.r_div_z;
        g_div_z_diff_1_2 = p2.g_div_z - p1.g_div_z;
        b_div_z_diff_1_2 = p2.b_div_z - p1.b_div_z;

        r_div_z_step_1_2 = r_div_z_diff_1_2 / num_steps_1_2;
        g_div_z_step_1_2 = g_div_z_diff_1_2 / num_steps_1_2;
        b_div_z_step_1_2 = b_div_z_diff_1_2 / num_steps_1_2;

        r_div_z_1_2 = p1.r_div_z;
        g_div_z_1_2 = p1.g_div_z;
        b_div_z_1_2 = p1.b_div_z;

        tex_x_div_z_diff_1_2 = p2.tex_x_div_z - p1.tex_x_div_z;
        tex_y_div_z_diff_1_2 = p2.tex_y_div_z - p1.tex_y_div_z;

        tex_x_div_z_step_1_2 = tex_x_div_z_diff_1_2 / num_steps_1_2;
        tex_y_div_z_step_1_2 = tex_y_div_z_diff_1_2 / num_steps_1_2;

        tex_x_div_z_1_2 = p1.tex_x_div_z;
        tex_y_div_z_1_2 = p1.tex_y_div_z;

        /*  Draw lower triangle. */
        for (int i = p1.y; i <= p2.y; i++) {
            /*  Draw row. */
            pixel_coord p_1_2 = {
                x_1_2,
                (double) i,
                inv_z_1_2,
                i_div_z_1_2,
                r_div_z_1_2,
                g_div_z_1_2,
                b_div_z_1_2,
                tex_x_div_z_1_2,
                tex_y_div_z_1_2
            };

            pixel_coord p_1_3 = {
                x_1_3,
                (double) i,
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
                    p_1_3,
                    bitmap_ptr,
                    buffer_width,
                    buffer_height
                );
            } else {
                draw_shaded_row(
                    window,
                    i,
                    p_1_3,
                    p_1_2,
                    bitmap_ptr,
                    buffer_width,
                    buffer_height
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
        inv_z_diff_2_3 = p3.inv_z - p2.inv_z;
        inv_z_step_2_3 = inv_z_diff_2_3 / num_steps_2_3;
        inv_z_2_3 = p2.inv_z;

        /*  Intensity / depth interpolation. */
        i_div_z_diff_2_3 = p3.i_div_z - p2.i_div_z;
        i_div_z_step_2_3 = i_div_z_diff_2_3 / num_steps_2_3;
        i_div_z_2_3 = p2.i_div_z;

        /*  Colour interpolation. */
        r_div_z_diff_2_3 = p3.r_div_z - p2.r_div_z;
        g_div_z_diff_2_3 = p3.g_div_z - p2.g_div_z;
        b_div_z_diff_2_3 = p3.b_div_z - p2.b_div_z;

        r_div_z_step_2_3 = r_div_z_diff_2_3 / num_steps_2_3;
        g_div_z_step_2_3 = g_div_z_diff_2_3 / num_steps_2_3;
        b_div_z_step_2_3 = b_div_z_diff_2_3 / num_steps_2_3;

        r_div_z_2_3 = p2.r_div_z;
        g_div_z_2_3 = p2.g_div_z;
        b_div_z_2_3 = p2.b_div_z;

        /*  Texture coordinates. */
        tex_x_div_z_diff_2_3 = p3.tex_x_div_z - p2.tex_x_div_z;
        tex_y_div_z_diff_2_3 = p3.tex_y_div_z - p2.tex_y_div_z;

        tex_x_div_z_step_2_3 = tex_x_div_z_diff_2_3 / num_steps_2_3;
        tex_y_div_z_step_2_3 = tex_y_div_z_diff_2_3 / num_steps_2_3;

        tex_x_div_z_2_3 = p2.tex_x_div_z;
        tex_y_div_z_2_3 = p2.tex_y_div_z;

        /*  Draw upper triangle. */
        for (int i = p2.y; i <= p3.y; i++) {
            /*  Draw row. */
            pixel_coord p_2_3 = {
                x_2_3,
                (double) i,
                inv_z_2_3,
                i_div_z_2_3,
                r_div_z_2_3,
                g_div_z_2_3,
                b_div_z_2_3,
                tex_x_div_z_2_3,
                tex_y_div_z_2_3
            };

            pixel_coord p_1_3 = {
                x_1_3,
                (double) i,
                inv_z_1_3,
                i_div_z_1_3,
                r_div_z_1_3,
                g_div_z_1_3,
                b_div_z_1_3,
                tex_x_div_z_1_3,
                tex_y_div_z_1_3
            };

            if (x_2_3 <= x_1_3) {
                p_1_3.x += 1;

                draw_shaded_row(
                    window,
                    i,
                    p_2_3,
                    p_1_3,
                    bitmap_ptr,
                    buffer_width,
                    buffer_height
                );
            } else {
                draw_shaded_row(
                    window,
                    i,
                    p_1_3,
                    p_2_3,
                    bitmap_ptr,
                    buffer_width,
                    buffer_height
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