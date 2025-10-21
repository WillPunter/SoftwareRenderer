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

    std::cout << "???" << std::endl;

    // std::cout << std::to_string(test_mesh->triangles[0].bitmap_ptr->width) << std::endl;
    // std::cout << std::to_string(test_mesh->triangles[0].bitmap_ptr->height) << std::endl;

    if (test_mesh == nullptr) {
        std::cerr << "Failed to load mesh." << std::endl;
        return -1;
    }

    Graphics::Model test_model {
        test_mesh,
        Maths::Vector<double, 4> { 0.0, -100.0, 0.0, 1.0 },
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

    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

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

        if (window->get_key(
            System::KeySymbol::SPACE) == System::KeyState::KEY_DOWN
        ) {
            /*  Compute camera vector. */
            Maths::Vector<double, 4> dir = {
                0.0, 0.0, 1.0, 0.0
            };

            auto mat = Maths::make_rotation_world(
                camera.rotation(0),
                camera.rotation(1),
                camera.rotation(2)
            );

            Maths::Vector<double, 4> delta = mat * dir;

            camera.position = camera.position + (4 * delta);
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

        /*  Draw bitmap. */
        /*
        for (int i = 0; i < bmp->height; i++) {
            for (int j = 0; j < bmp->width; j++) {
                int pixel_index = i * bmp->width + j;
                Graphics::draw_pixel(
                    *window,
                    bmp_x + j,
                    bmp_y + i,
                    bmp->pixels[pixel_index].r,
                    bmp->pixels[pixel_index].g,
                    bmp->pixels[pixel_index].b
                );
            }
        }
        */

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
        );
        */

        window->display_render_buffer();
    }

    delete test_mesh;
}