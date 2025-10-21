/*  Worlds demo.

    This demo allows you to explore a variety of maps using a
    first-person-view camera. */

#include "./../../src/System/RenderWindow.hpp"
#include "./../../src/Graphics/Rasteriser.hpp"
#include "./../../src/Graphics/Model.hpp"
#include "./../../src/Graphics/Renderer.hpp"
#include "./../../src/Resources/load_resources.hpp"

#include <iostream>
#include <string>
#include <chrono>

double rotation_speed = 4.0;
double move_speed = 10.0;

int main() {
    /*  Load bitmap. */
    Resources::TrueColourBitmap* bmp = Resources::load_bitmap_from_file("./../res/artisans_hub_texture.bmp");

    if (bmp) {
        std::cout << "Loaded bitmap with width "
            << std::to_string(bmp->width) << " and height "
            << std::to_string(bmp->height) << "." << std::endl;
    } else {
        return -1;
    }

    std::cout << std::to_string(bmp->width) << std::endl;
    std::cout << std::to_string(bmp->height) << std::endl;

    int bmp_x = 32;
    int bmp_y = 48;

    /*  pixels[num_pixels]Create window. */
    std::unique_ptr<System::RenderWindow> window(
        System::make_render_window("Worlds", 640, 480));

    Graphics::Mesh* test_mesh =
        Resources::load_mesh_from_obj("./../res/test.obj");
    
    Resources::attach_texture(*test_mesh, *bmp);

    if (test_mesh == nullptr) {
        std::cerr << "Failed to load mesh." << std::endl;
        return -1;
    }

    Graphics::Model test_model {
        test_mesh,
        Maths::Vector<double, 4> { 0.0, -20.0, 0.0, 1.0 },
        Maths::Vector<double, 4> { 1.0, 1.0, 1.0, 0.0 },
        Maths::Vector<double, 4> { 0.0, 0.0, 0.0, 0.0 }
    };

    std::vector<Graphics::Light> lights {
        Graphics::Light {
            Graphics::LightType::AMBIENT,
            0.5,
            Maths::Vector<double, 4> { 0.0, 0.0, 0.0, 0.0 }
        },

        Graphics::Light {
            Graphics::LightType::DIRECTION,
            0.5,
            Maths::Vector<double, 4> { 1.0, -2.0, -1.0, 0.0 }
        }
    };

    Graphics::Renderer renderer(45.0, 640.0 / 480.0, 1000.0);

    Graphics::Camera camera;

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    double delta_time = 0.0;

    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

        //if (window->get_key(32) == System::KeyState::KEY_DOWN) {
        //    camera.rotation(2) += 0.1;
        //}

        if (window->get_key(
            System::KeySymbol::ARROW_LEFT) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(1) += rotation_speed * delta_time;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_RIGHT) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(1) -= rotation_speed * delta_time;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_UP) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(0) -= rotation_speed * delta_time;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_DOWN) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(0) += rotation_speed * delta_time;
        }

        if (window->get_key(
            System::KeySymbol::SPACE) == System::KeyState::KEY_DOWN
        ) {
            /*  Compute camera vector. */
            Maths::Vector<double, 4> dir = {
                0.0, 0.0, 1.0, 0.0
            };

            auto mat = Maths::make_inverse_rotation_world(
                -camera.rotation(0),
                -camera.rotation(1),
                -camera.rotation(2)
            );

            Maths::Vector<double, 4> delta = mat * dir;

            camera.position = camera.position + (move_speed * delta_time * delta);
        }

        /*  Construct scene. */
        Graphics::Scene scene {
            std::vector<Graphics::Model*> { &test_model },
            lights,
            camera
        };

        for (int i = 0; i < window->get_width(); i++) {
            for (int j = 0; j < window->get_height(); j++) {
                Graphics::draw_pixel(
                    *window,
                    i,
                    j,
                    0,
                    120,
                    255
                );
            }
        };

        /*  Render scene. */
        renderer.render_scene(*window, scene);

        window->display_render_buffer();

        /*  Compute end time. */
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_diff = end - start;
        delta_time = time_diff.count();
        start = end;
    }

    delete test_mesh;
}