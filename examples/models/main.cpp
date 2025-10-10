/*  models/main.cpp

    The creation of a simple triangle model that is made to rotate. */

#include "./../../src/System/RenderWindow.hpp"
#include "./../../src/Graphics/Rasteriser.hpp"
#include "./../../src/Graphics/Model.hpp"
#include "./../../src/Graphics/Renderer.hpp"

#include <iostream>
#include <string>

int main() {
    /*  Create window. */
    std::unique_ptr<System::RenderWindow> window(
        System::make_render_window("Simple pixels!!!", 640, 480));
    
    Graphics::Triangle test_triangle {
        Maths::Vector<double, 4>{-1.0, 0.0, 0.0, 1.0},
        Maths::Vector<double, 4>{0.0, 3.0, 0.0, 1.0},
        Maths::Vector<double, 4>{1.0, 0.0, 0.0, 1.0}
    };

    Graphics::Mesh test_mesh {
        std::vector<Graphics::Triangle> { test_triangle }
    };

    Graphics::Model test_model {
        &test_mesh,
        Maths::Vector<double, 4> { -1.0, 0.0, 10.0, 1.0 },
        Maths::Vector<double, 4> { 1.0, 1.0, 1.0, 0.0 },
        Maths::Vector<double, 4> { 0.0, 0.0, 0.0, 0.0 }
    };

    Graphics::Renderer renderer(45.0, 480.0 / 640.0);

    while (window->is_open()) {
        window->handle_events();

        window->clear_window();

        test_model.rotation(1) += 0.001;

        /*  Transform to world space. */
        Maths::Matrix<double, 4, 4> model_world_transform = Graphics::model_transform(test_model);

        /*  Create new transformed triangle. */
        Graphics::Triangle world_triangle;

        for (int i = 0; i < 3; i++) {
            world_triangle.points[i] = model_world_transform * (test_model.mesh->triangles[0].points[i]);
        }

        /*  Project onto viewing plane. */
        Graphics::Triangle projected_triangle;

        for (int i = 0; i < 3; i++) {
            projected_triangle.points[i] = renderer.project_vertex(world_triangle.points[i]);
        }

        /*  Convert to pixel space. */
        Graphics::pixel_coord pixels[3];

        for (int i = 0; i < 3; i++) {
            pixels[i] = {
                (int) ((projected_triangle.points[i](0) + 1) * 319),
                (int) ((projected_triangle.points[i](1) + 1) * 239)
            };
        }

        Graphics::draw_wireframe_triangle(*window, pixels[0], pixels[1], pixels[2], 255, 0, 0);

        window->display_render_buffer();
    }
}