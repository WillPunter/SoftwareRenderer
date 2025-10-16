/*  X11RGBARenderWindow.cpp */

#include "X11RGBARenderWindow.hpp"
#include <cstring>
#include <iostream>

namespace System {

X11RGBARenderWindow::X11RGBARenderWindow(std::string title, int width,
    int height) : window{title, width, height}, rgba_buffer(width * height) {
    /*  Create graphics context for window - use default mask and metadata
        values (two zero parameters). */
    this->graphics_context = XCreateGC(this->window.server_connection,
        this->window.window, 0, 0);
    
    /*  Create XImage structure - bitmap metadata to inform window of how to
        interpret render buffer when blitting with an XPutImage call. */
    this->image_data = XCreateImage(
        this->window.server_connection, /* display (server connection). */
        this->window.visual_info, /* visual info / metadata. */
        this->TRUE_COLOR_BIT_DEPTH, /* bit depth - 24 bits for true colour. */
        ZPixmap, /* format of RGB bitmap. */
        0, /* offset from start of buffer to first colour value. */
        (char*) this->rgba_buffer.data(), /* pointer to data. */
        this->window.width, this->window.height, /* width and height. */
        32, /*  pad to nearest 32 bits. */
        0 /* bytes per line - 0 indicates default width * sizeof(padded pixel)
             calculation is used. */
    );

    /*  Calculate shift of red, green and blue values. */
    this->red_shift = this->compute_shift_from_rgb_mask(
        this->window.visual_info->red_mask);
    
    this->green_shift = this->compute_shift_from_rgb_mask(
        this->window.visual_info->green_mask);
    
    this->blue_shift = this->compute_shift_from_rgb_mask(
        this->window.visual_info->blue_mask);
}

bool X11RGBARenderWindow::handle_events() {
    return this->window.handle_events();
}

void X11RGBARenderWindow::close_window() {
    this->window.close_window();
}

bool X11RGBARenderWindow::is_open() {
    return this->window.is_open();
}

void X11RGBARenderWindow::clear_window() {
    std::memset(this->rgba_buffer.data(), 0, this->window.width *
        this->window.height * sizeof(pixel));
}

void X11RGBARenderWindow::display_render_buffer() {
    XPutImage(this->window.server_connection, this->window.window,
        this->graphics_context, this->image_data, 0, 0, 0, 0,
        this->window.width, this->window.height);
    
    XFlush(this->window.server_connection);
}

inline void X11RGBARenderWindow::draw_pixel(int x, int y, uint8_t red,
    uint8_t green, uint8_t blue) {
    uint32_t pixel_val = (red << this->red_shift) |
        (green << this->green_shift) | (blue << this->blue_shift);

    this->rgba_buffer[y * this->window.width + x] = pixel_val;
}

int X11RGBARenderWindow::get_width() {
    return this->window.width;
}

int X11RGBARenderWindow::get_height() {
    return this->window.height;
}

KeyState X11RGBARenderWindow::get_key(KeySymbol key_id) {
    return window.get_key(key_id);
}

/*  Prerequisite - rgb mask is of form 0b0...1...1...0, i.e. a string of 1s
    surrounded by zero or more 0's on each side. */
uint8_t X11RGBARenderWindow::compute_shift_from_rgb_mask(
    unsigned long rgb_mask) {
    uint8_t count = 0;

    while ((rgb_mask & 1) != 1 && count <
        sizeof(this->window.visual_info->red_mask) * 8) {
        rgb_mask >>= 1;
        count ++;
    }

    return count;
}

}