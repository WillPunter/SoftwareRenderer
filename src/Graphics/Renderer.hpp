/*  Renderer.hpp

    Stores renderer state (e.g. FOV, aspect ratio, etc.) and performs
    perspective projections using it.
    
    The Renderer is responsible for 3d graphics manipulation. It is not
    responsible for 2d primitive rendering - see the Rasteriser for this. */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "./../Maths/Vector.hpp"
#include "Model.hpp"

#include <list>
#include <vector>

namespace Graphics {

struct Scene {
    std::vector<Model*> models;
};

class Renderer {
    public:
        Renderer(double fov, double aspect_ratio, double far_plane_distance);

        void render_scene(const Scene& scene);

    private:
        double get_sign(double val);

        int line_plane_intersection(
            Maths::Vector<double, 4> line_point,
            Maths::Vector<double, 4> line_direction,
            Maths::Vector<double, 4> plane_point,
            Maths::Vector<double, 4> plane_direction_1,
            Maths::Vector<double, 4> plane_direction_2,
            Maths::Vector<double, 4>& out
        );
        
        Triangle transform_triangle(
            const Triangle& triangle,
            const Maths::Matrix<double, 4, 4>& transform
        );

        void build_triangles_list_from_models(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles,
            std::vector<Model*>& models
        );

        void convert_triangles_to_camera_space(
            std::vector<Triangle>& triangles
        );

        void cull_triangle_back_faces(std::vector<Triangle>& triangles);

        void clip_plane(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles,
            const Maths::Vector<double, 4>& point_on_plane,
            const Maths::Vector<double, 4>& direction_1,
            const Maths::Vector<double, 4>& direction_2
        );

        void clip_near_plane(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles
        );

        void clip_far_plane(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles
        );

        void clip_near_far_plane(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles
        );
        
        void perspective_project_triangles(
            std::list<Triangle*>& active_triangles
        );

        void clip_triangles(
            std::vector<Triangle>& triangles,
            std::list<Triangle*>& active_triangles
        );
    
        void rasterise_triangles(
            std::list<Triangle*>& active_triangles
        );

        double fov;
        double aspect_ratio;
        double view_plane_distance;
        double far_plane_distance;
};

}

#endif