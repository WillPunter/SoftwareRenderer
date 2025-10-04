/*  Window_x11.hpp */

#ifndef WINDOW_X11_HPP
#define WINDOW_X11_HPP

#include <X11/Xlib.h>

#include "./../Window.hpp"

namespace System_Linux {

class Window_x11 : public System::Window {
    public:
        Window_x11() = delete;

        void draw_pixel(int x, int y, uint8_t red, uint8_t green,
            uint8_t blue) override;

        /*  make_unique function is made "friend" as it should be the only
            means of constructing a Window_x11 type. This type is not meant to
            constitute part of the graphics engine API, but rather to
            encapsulate the bare minimum Linux X11 functionality to create a
            window and render buffer and implement the System::Window interface
            to them. */
        friend std::unique_ptr<System::Window> System::make_window(
            std::string, int, int);
        
    private:
        /*  Private constructor to enable */
        Window_x11(std::string title, int width, int height);

        std::string title;
        int width;
        int height;
        
        Display* display;

        using X11_Window = ::Window;
        X11_Window window;

        uint32_t* render_buffer;
};

}

#endif