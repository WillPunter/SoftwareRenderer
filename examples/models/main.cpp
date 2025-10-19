/*  models/main.cpp

    The creation of a simple triangle model that is made to rotate. */

#include "./../../src/System/RenderWindow.hpp"
#include "./../../src/Graphics/Rasteriser.hpp"
#include "./../../src/Graphics/Model.hpp"
#include "./../../src/Graphics/Renderer.hpp"
#include "./../../src/Resources/load_mesh.hpp"

#include <iostream>
#include <string>

int main() {
    /*  Create window. */
    std::unique_ptr<System::RenderWindow> window(
        System::make_render_window("Models", 640, 480));
    
    Graphics::Triangle test_triangle {
        Maths::Vector<double, 4>{-1.0, 0.0, 0.0, 1.0},
        Maths::Vector<double, 4>{0.0, 3.0, 0.0, 1.0},
        Maths::Vector<double, 4>{1.0, 0.0, 0.0, 1.0}
    };

    //Graphics::Mesh test_mesh {
    //    std::vector<Graphics::Triangle> { test_triangle }
    //};

    Graphics::Mesh* test_mesh =
        Resources::load_mesh_from_obj("./../res/cow.obj");

    if (test_mesh == nullptr) {
        std::cerr << "Failed to load mesh." << std::endl;
        return -1;
    }

    Graphics::Model test_model {
        test_mesh,
        Maths::Vector<double, 4> { 0.0, 0.0, 7.0, 1.0 },
        Maths::Vector<double, 4> { 1.0, 1.0, 1.0, 0.0 },
        Maths::Vector<double, 4> { 0.0, 0.0, 0.0, 0.0 }
    };

    Graphics::Renderer renderer(45.0, 640.0 / 480.0, 1000.0);

    Graphics::Camera camera;

    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

        test_model.rotation(1) += 0.01;
        test_model.rotation(2) += 0.005;

        //if (window->get_key(32) == System::KeyState::KEY_DOWN) {
        //    camera.rotation(2) += 0.1;
        //}

        if (window->get_key(
            System::KeySymbol::ARROW_LEFT) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(1) += 0.01;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_RIGHT) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(1) -= 0.01;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_UP) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(0) += 0.01;
        }

        if (window->get_key(
            System::KeySymbol::ARROW_DOWN) == System::KeyState::KEY_DOWN
        ) {
            camera.rotation(0) -= 0.01;
        }

        /*  Construct scene. */
        Graphics::Scene scene {
            std::vector<Graphics::Model*> { &test_model },
            camera
        };

        /*  Render scene. */
        renderer.render_scene(*window, scene);

        /*  Draw triangle */
        /*
        Graphics::draw_shaded_triangle(
            *window,
            Graphics::pixel_coord  {
                600,  // x
                300,  // y
                1.0, // depth
                1.0, // intensity
                255, // r
                0,   // g
                0,   // b
                1,   // tex x
                1    // tex y
            },

            Graphics::pixel_coord  {
                200, // x
                144,  // y
                1.0, // depth
                1.0, // intensity
                0, // r
                255,   // g
                0,   // b
                1,   // tex x
                1    // tex y
            },

            Graphics::pixel_coord  {
                100, // x
                32, // y
                1.0, // depth
                1.0, // intensity
                0,   // r
                0,   // g
                255, // b
                1,   // tex x
                1    // tex y
            }
        );*/

        window->display_render_buffer();
    }

    delete test_mesh;
}