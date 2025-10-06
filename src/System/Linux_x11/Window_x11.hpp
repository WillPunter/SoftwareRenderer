/*  Window_x11.hpp */

#ifndef WINDOW_X11_HPP
#define WINDOW_X11_HPP

#include <X11/Xlib.h>
#include <vector>

#include "./../Window.hpp"

namespace System_Linux {

class Window_x11 : public System::Window {
    public:
        Window_x11() = delete;

        bool handle_events() override;

        void clear_screen() override;

        void draw_pixel(int x, int y, uint8_t red, uint8_t green,
            uint8_t blue) override;
        
        void draw_rectangle(int x0, int y0, int x1, int y1, uint8_t red, uint8_t green, uint8_t blue) override;
        
        void update_buffer() override;
        
        void close_window() override;

        bool is_closed() const override;

        /*  make_unique function is made "friend" as it should be the only
            means of constructing a Window_x11 type. This type is not meant to
            constitute part of the graphics engine API, but rather to
            encapsulate the bare minimum Linux X11 functionality to create a
            window and render buffer and implement the System::Window interface
            to them. */
        friend std::unique_ptr<System::Window> System::make_window(
            std::string, int, int, int);
        
    private:
        bool open = true;
        
        /*  Private constructor - construction should only be done by the
            make_window factory function, so as to encapsulate platform
            specific details. */
        Window_x11(std::string title, int width, int height, int scaling);

        bool multiplex_event(XEvent* event);

        static int rgb_mask_shift(unsigned long mask);

        std::string title;
        int width;
        int height;
        
        Display* display;

        using X11_Window = ::Window;
        X11_Window window;

        Atom window_manager_destroy_window_id;

        union pixel {
            /*  Typical RGB linux for X11 - may need adjustment on other
                systems. If in doubt, just use the provided masks in the Visual
                structure.
                
                Note that this is just a logical RGBX / RGBA (alpha is unused
                however, so X is often used as a placeholder) but in little
                endian. */
            struct {
                uint8_t blue;
                uint8_t green;
                uint8_t red;
                uint8_t x;
            } rgb;

            uint32_t data;
        };

        std::vector<pixel> render_buffer;
        std::vector<pixel> display_buffer;

        int scaling;

        XImage* image_descriptor;

        GC graphics_context;
};

}

#endif