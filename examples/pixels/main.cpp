/*  pixels/main.cpp

    The creation of a simple window with random pixels displaying to the
    screen. */

#include "./../../src/System/RenderWindow.hpp"

#include <string>

int main() {
    /*  Create window. */
    std::unique_ptr<System::RenderWindow> window =
        System::make_render_window("Simple pixels!!!", 640, 480);
    
    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                window->draw_pixel(i, j, 255 - i, 0, j);
            }
        }

        window->display_render_buffer();
    }
}