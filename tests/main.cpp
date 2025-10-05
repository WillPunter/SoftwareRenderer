/*  main.cpp */

#include "./../src/System/Window.hpp"

int main() {
    std::unique_ptr<System::Window> window = System::make_window("Hello world!!!", 640, 480);

    while (!window->is_closed()) {
        window->handle_events();
    };

    return 0;
}