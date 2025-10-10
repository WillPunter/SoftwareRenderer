/*  Renderer.cpp

    Implements 3d rendering functionality. */

#include "Renderer.hpp"
#include <cmath>

namespace Graphics {

Renderer::Renderer(double fov, double aspect_ratio) : fov{fov},
    aspect_ratio{aspect_ratio}, view_plane_distance{1.0 / tan(fov)} {};

Maths::Vector<double, 4> Renderer::project_vertex(
    Maths::Vector<double, 4> vector) {
    return Maths::Vector<double, 4> {
        vector(0) * this->view_plane_distance / vector(2),
        vector(1) * this->view_plane_distance / vector(2) * this->aspect_ratio,
        this->view_plane_distance,
        vector(3)
    };
}

}