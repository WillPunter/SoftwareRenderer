/*  lines/main.cpp

    The creation of a simple window with several lines. */

#include "./../../src/System/RenderWindow.hpp"
#include "./../../src/Graphics/Rasteriser.hpp"

#include <string>

int main() {
    /*  Create window. */
    std::unique_ptr<System::RenderWindow> window(
        System::make_render_window("Simple pixels!!!", 640, 480));
    
    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

        Graphics::draw_line(*window, { 100, 100 }, { 300, 200 }, 255, 0, 0);

        Graphics::draw_wireframe_triangle(*window, { 100, 100 }, { 200, 300 }, {300, 150}, 0, 255, 0);

        window->display_render_buffer();
    }
}