/*  main.cpp */

#include "./../src/System/Window.hpp"
#include <iostream>
#include <chrono>

int main() {
    int width = 320;
    int height = 240;
    std::unique_ptr<System::Window> window = System::make_window("Hello world!!!", width, height, 2);

    int x = 0;
    int y = 100;
    int box_width = 180;
    int box_height = 180;

    int frames = 0;
    constexpr int frame_count = 1000;

    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time;

    int num_rectangles = 500;

    while (!window->is_closed()) {
        bool frame_continue = window->handle_events();

        if (frame_continue) {
            window->clear_screen();

            /*
            for (int i = 0; i < num_rectangles; i++) {
                int x_coord = 0;
                int y_coord = 0;

                int box_width = 32;
                int box_height = 32;

                uint8_t red = 255;
                uint8_t green = 0;
                uint8_t blue = 0;

                for (int i = x_coord; i < x_coord + box_width && i < width; i++) {
                    for (int j = y_coord; j < y_coord + box_height && j < height; j++) {
                        window->draw_pixel(i, j, red, green, blue);
                    }
                }
            }
            */

            for (int i = 0; i < num_rectangles; i++) {
                window->draw_rectangle(0, 0, 31, 31, 255, 0, 0);
            }


            window->update_buffer();

            frames++;

            if (frames == frame_count) {
                end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end_time - start_time;

                double seconds = elapsed.count();

                std::cout << "FPS = " << std::to_string(frame_count / seconds) << "." << std::endl;
                
                start_time = std::chrono::high_resolution_clock::now();

                frames = 0;
            }
        }
    };

    return 0;
}