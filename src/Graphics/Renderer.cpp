/*  Renderer.cpp

    Implements 3d rendering functionality. */

#include "Renderer.hpp"
#include "./../Maths/Transform.hpp"
#include "Rasteriser.hpp"
#include <cmath>
#include <list>
#include <iterator>

namespace Graphics {

Renderer::Renderer(double fov, double aspect_ratio, double far_plane_distance)
    : fov{fov}, aspect_ratio{aspect_ratio},
    view_plane_distance{1.0 / tan(fov)}, far_plane_distance{},
    screen_left_bound { -1.0 },
    screen_right_bound { 1.0 },
    screen_top_bound { 1.0 / aspect_ratio },
    screen_bottom_bound { -1.0 / aspect_ratio } {};

void Renderer::render_scene(
    System::RenderWindow& render_window,
    const Scene& scene
) {
    /*  Note that this is an inefficient solution - get it working first, then
        optimise the rendering pipeline. In particular think about how:
            We can avoid making redundant copies of triangles - how can we add
            and remove triangles from the container quickly (ideally in constant
            time) to support clipping.
            
        Ideal operation:
            Extract triangles from models in worldspace (necessary copies must
            be constructed as meshes cannot be modified).
            
            Modify triangles by transforming with respect to camera space.
            
            Clip triangles against back plane - remove, add and modify
            triangles as necessary (how can we do this fast?).
            
            Project all triangles onto the viewing plane.
            
            Clip all triangles against screen bounds - remove, add and
            modify triangles as necessary (also think about how this can be
            done fast).
        
        Optimisation ideas:
            Maintain a triangle buffer of all possible triangles used. New
            triangles may be added to this, but never removed (a vector would
            work well for this with sufficient space reserved).
            Then keep a list of pointers to active triangles (need a structure
            which can be iterated in O(n) for which active triangles can be
            easily added and removed - a linked list would be superb for this
            as order of triangles does not matter). */

    /*  We currently have a ist of pointers to models. We want to build a list
        of all triangles in worldspace (copies). */
    std::vector<Triangle> triangles;
    std::list<int> active_indices;

    size_t triangle_count = 0;

    for (const Model* m : scene.models) {
        /*  Transform with respect to world space. */
        Maths::Matrix<double, 4, 4>  matrix_model = model_transform(*m);

        /*  Transform triangles to worldspace. */
        for (const Triangle& t : m->mesh->triangles) {
            triangles.push_back(this->transform_triangle(t, matrix_model));
            active_indices.push_back(triangle_count);
            triangle_count ++;
        }
    }

    /*  Transform triangles to camera space - TODO. */

    /*  Compute lighting at each vertex (Gouraud Shading) - TODO. */

    /*  Clip against near plane in 3d. */
    this->clip_near_plane(triangles, active_indices);

    /*  Project triangles into clip space - preserving z coordinate for
        depth comparisons and comparing against the near and far planes. */
    this->perspective_project_triangles(triangles, active_indices);

    /*  Clip triangles against screen bounds. */
    this->clip_screen_bounds(triangles, active_indices);

    /*  Convert triangles to pixel space. */
    this->convert_triangles_to_pixel_space(
        triangles,
        active_indices,
        render_window.get_width(),
        render_window.get_height()
    );

    /*  Raterise triangles. */
    this->rasterise_triangles(render_window, triangles, active_indices);
}

inline Triangle Renderer::transform_triangle(
    const Triangle& triangle,
    const Maths::Matrix<double, 4, 4>& transform
) {
    Triangle res;

    for (int i = 0; i < 3; i++) {
        res.points[i] = transform * triangle.points[i];
    }

    return res;
}

void Renderer::build_triangles_list_from_models(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices,
    std::vector<Model*>& models
) {
    for (const Model* m : models) {
        Maths::Matrix<double, 4, 4> model_transform_matrix
            = model_transform(*m);
        
        for (const Triangle& t : m->mesh->triangles) {
            triangles.push_back(transform_triangle(t, model_transform_matrix));
            active_indices.push_back(triangles.size());
        }
    }
}

void Renderer::convert_triangles_to_camera_space(
    std::vector<Triangle>& triangles
) {
   /*  TODO - implement with camera structure. */
}

void Renderer::cull_triangle_back_faces(std::vector<Triangle>& triangles) {
    /*  TODO - implement using vector and scalar product to check direction
    relative to viewing direction (Z+). */
}

/*  Make triangles from vertex array - assumes that vertices are in
    an order such that their traversal in order would form a convex
    polygon. */
int Renderer::make_triangles(
    int num_vertices,
    Maths::Vector<double, 4> in_vertices[4],
    Triangle out_triangles[2]
) {
    int vertex_counter = 0;
    int triangle_count = 0;

    /*  Form a triangle out of the first vertex and every subsequent
        pair of directly connected vertices, so v1 -> v2 -> v3 -> v4
        becomes (v1 -> v2 -> v3), (v1 -> v3 -> v4). Since the order
        of each pair of consecutive vertices is preserved, and we
        simply add v1, this innately preserves the ordering / winding
        of the shape. */
    for (int i = 1; i < num_vertices - 1; i++) {
        out_triangles[triangle_count].points[0] = in_vertices[0];
        out_triangles[triangle_count].points[1] = in_vertices[i];
        out_triangles[triangle_count].points[2] = in_vertices[i + 1];

        triangle_count ++;
    }

    return triangle_count;
}

/*  To clip the active triangles against the near plane, we invoke the
    clip_triangles member function with lambdas for determining whether a
    vertex is in the viewing plane and for finding intersects with the
    viewing plane. Note that the terms "viewing plane" and "near plane"
    refer to the same thing. */
void Renderer::clip_near_plane(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    clip_triangles(
        triangles,
        active_indices,

        /*  Lambda for in_viewing_region operand. In this case the viewing
            region is the forward side of the near plane (z > view_distance). */
        [this](Maths::Vector<double, 4> vertex) {
            return vertex(2) >= this->view_plane_distance;
        },

        /*  Lambda for get_intersect operand. This returns a vector storing the
            point where the line vertex_1 -> vertex_2 intersects the
            viewing region boundary. That is a precondition of this function
            and calls on vertex values that do not establish this precondition
            should be considered undefined as this code needs to be called for
            all triangles, and validation per triangle would cost clock
            cycles. */
        [this](
            Maths::Vector<double, 4> vertex_1,
            Maths::Vector<double, 4> vertex_2
        ) {
            Maths::Vector<double, 4> diff = vertex_2 - vertex_1;
            double scale = (this->view_plane_distance - vertex_1(2)) / diff(2);
            return vertex_1 + scale * diff;
        }
    );
}

/*  For now, we apply perspective projection using the following method:
        Consider a point (x, y, z) in 3d space. Suppose that there is a line
        from the camera (i.e. the origin, in camera space) to the point that
        passes through the near plane (z = z_near) at coordinates:
        (x', y', z_near). Then, this new point is the perspective projection
        of the original point. As (x', y', z_near) and (x, y, z) both fall on
        the same line that passes through the origin, they form similar
        triangles with the origin. Hence:
            x' / z_near = x / z
        So:
            x' = x * z_near / z
        And similarly
            y' = y * z_near / z
    In a geometrically pure projection, we would also move the z coordinate to
    the near plane (z_near) but we do not do this as we will want to know the
    depths at the per-pixel level when we come to do tetxure sampling and
    shading interpolation.
    
    In the future it might be worth considering a homogeneous projection like
    in hardware accelerated APIs? This might just introduced more floating
    point operations in software though (which do not translate to time
    overhead in hardware due to the massive level of parallelism). */
void Renderer::perspective_project_triangles(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    /*  Project all vertices. */
    std::list<int>::iterator itr = active_indices.begin();
    
    while (itr != active_indices.end()) {
        Triangle* curr_tri = &triangles[*itr];

        /*  Project all vertices. */
        for (int i = 0; i < 3; i++) {
            double z_near_div_z = this->view_plane_distance /
                curr_tri->points[i](2);
            curr_tri->points[i](0) *= z_near_div_z;
            curr_tri->points[i](1) *= z_near_div_z;
        }

        itr++;
    }
}

/*  Clip in 2d against the left bound of the screen. */
void Renderer::clip_left_bound(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_triangles(
        triangles,
        active_indices,

        [this](Maths::Vector<double, 4>& vertex) {
            return vertex(0) > this->screen_left_bound;
        },

        [this](
            Maths::Vector<double, 4>& vertex_1,
            Maths::Vector<double, 4>& vertex_2
        ) {
            Maths::Vector<double, 4> diff = vertex_2 - vertex_1;
            double scale = (this->screen_left_bound - vertex_1(0)) / diff(0);
            return vertex_1 + scale * diff;
        }
    );
}

/*  Clip in 2d against the right bound of the screen. */
void Renderer::clip_right_bound(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_triangles(
        triangles,
        active_indices,

        [this](Maths::Vector<double, 4>& vertex) {
            return vertex(0) < this->screen_right_bound;
        },

        [this](
            Maths::Vector<double, 4>& vertex_1,
            Maths::Vector<double, 4>& vertex_2
        ) {
            Maths::Vector<double, 4> diff = vertex_2 - vertex_1;
            double scale = (this->screen_right_bound - vertex_1(0)) / diff(0);
            return vertex_1 + scale * diff;
        }
    );
}

/*  Clip in 2d against the top bound of the screen. */
void Renderer::clip_top_bound(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_triangles(
        triangles,
        active_indices,

        [this](Maths::Vector<double, 4>& vertex) {
            return vertex(1) < this->screen_top_bound;
        },

        [this](
            Maths::Vector<double, 4>& vertex_1,
            Maths::Vector<double, 4>& vertex_2
        ) {
            Maths::Vector<double, 4> diff = vertex_2 - vertex_1;
            double scale = (this->screen_top_bound - vertex_1(1)) / diff(1);
            return vertex_1 + scale * diff;
        }
    );
}

/*  Clip in 2d against the bottom bound of the screen. */
void Renderer::clip_bottom_bound(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_triangles(
        triangles,
        active_indices,

        [this](Maths::Vector<double, 4>& vertex) {
            return vertex(1) > this->screen_bottom_bound;
        },

        [this](
            Maths::Vector<double, 4>& vertex_1,
            Maths::Vector<double, 4>& vertex_2
        ) {
            Maths::Vector<double, 4> diff = vertex_2 - vertex_1;
            double scale = (this->screen_bottom_bound - vertex_1(1)) / diff(1);
            return vertex_1 + scale * diff;
        }
    );
}

/*  Clip against screen bounds - in 2d. */
void Renderer::clip_screen_bounds(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_left_bound(triangles, active_indices);
    this->clip_right_bound(triangles, active_indices);
    this->clip_top_bound(triangles, active_indices);
    this->clip_bottom_bound(triangles, active_indices);
}

/*  Convert triangles to pixel space. */
void Renderer::convert_triangles_to_pixel_space(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices,
    int buffer_width,
    int buffer_height
) {
    std::list<int>::iterator itr = active_indices.begin();

    while (itr != active_indices.end()) {
        Triangle* curr_triangle = &triangles[*itr];

        for (int i = 0; i < 3; i++) {
            curr_triangle->points[i](0) = round(
                ((curr_triangle->points[i](0) - this->screen_left_bound) /
                (this->screen_right_bound - this->screen_left_bound)) *
                (buffer_width - 1)
            );
            
            curr_triangle->points[i](1) = (buffer_height - 1) - round(
                ((curr_triangle->points[i](1) - this->screen_bottom_bound) /
                (this->screen_top_bound - this->screen_bottom_bound)) *
                (buffer_height - 1)
            );
        }

        itr ++;
    }
}

/*  Rasterise triangles - for now, we simply just invoke the wireframe triangle
    function from the rasteriser. */
void Renderer::rasterise_triangles(
    System::RenderWindow& render_window,
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    std::list<int>::iterator itr = active_indices.begin();

    while (itr != active_indices.end()) {
        Triangle* curr_triangle = &triangles[*itr];

        draw_wireframe_triangle(
            render_window,

            {
                (int) floor(curr_triangle->points[0](0)),
                (int) floor(curr_triangle->points[0](1))
            },

            {
                (int) floor(curr_triangle->points[1](0)),
                (int) floor(curr_triangle->points[1](1))
            },

            {
                (int) floor(curr_triangle->points[2](0)),
                (int) floor(curr_triangle->points[2](1))
            },

            255, 0, 0
        );

        itr ++;
    }
}

}