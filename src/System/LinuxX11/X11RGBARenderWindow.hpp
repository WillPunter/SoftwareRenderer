/*  X11RGBARenderWindow.hpp */

#ifndef X11RGBARENDER_WINDOW_HPP
#define X11RGBARENDER_WINDOW_HPP

#include <vector>
#include "./../RenderWindow.hpp"
#include "X11Window.hpp"

namespace System {

class X11RGBARenderWindow : public RenderWindow {
    public:
        X11RGBARenderWindow() = delete;

        bool handle_events() override;

        void close_window() override;

        bool is_open() override;

        void clear_window() override;

        void display_render_buffer() override;

        void draw_pixel(int x, int y, uint8_t red, uint8_t green,
            uint8_t blue) override;

        /*  Only allow public construction through non-member factory method
            make_render_window. */
        friend std::unique_ptr<RenderWindow> make_render_window(
            std::string title, int width, int height);

    private:
        X11RGBARenderWindow(std::string title, int width, int height);

        uint8_t compute_shift_from_rgb_mask(unsigned long rgb_mask);

        X11Window window;

        using pixel = uint32_t;
        std::vector<pixel> rgba_buffer;

        static constexpr int TRUE_COLOR_BIT_DEPTH = 24;

        GC graphics_context;
        XImage* image_data;

        uint8_t red_shift;
        uint8_t green_shift;
        uint8_t blue_shift;
};

}

#endif