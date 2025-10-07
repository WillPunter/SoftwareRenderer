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

        void update_buffer() override;
        
        void close_window() override;

        bool is_closed() const override;

        std::vector<System::pixel>& get_render_buffer() override;

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

        std::vector<System::pixel> render_buffer;
        std::vector<System::pixel> display_buffer;

        int scaling;

        XImage* image_descriptor;

        GC graphics_context;
};

}

#endif