/*  Renderer.hpp

    Stores renderer state (e.g. FOV, aspect ratio, etc.) and performs
    perspective projections using it.
    
    The Renderer is responsible for 3d graphics manipulation. It is not
    responsible for 2d primitive rendering - see the Rasteriser for this. */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "./../Maths/Vector.hpp"

namespace Graphics {

class Renderer {
    public:
        Renderer(double fov, double aspect_ratio);

        Maths::Vector<double, 4> project_vertex(
            Maths::Vector<double, 4> vector);

    private:
        double fov;
        double aspect_ratio;
        double view_plane_distance;
};

}

#endif