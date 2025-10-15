/*  Renderer.cpp

    Implements 3d rendering functionality. */

#include "Renderer.hpp"
#include "./../Maths/Transform.hpp"
#include <cmath>
#include <list>
#include <iterator>

namespace Graphics {

Renderer::Renderer(double fov, double aspect_ratio, double far_plane_distance)
    : fov{fov}, aspect_ratio{aspect_ratio},
    view_plane_distance{1.0 / tan(fov)}, far_plane_distance{} {};

void Renderer::render_scene(const Scene& scene) {
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
    std::list<Triangle*> active_triangles;

    size_t triangle_count = 0;

    for (const Model* m : scene.models) {
        /*  Transform with respect to world space. */
        Maths::Matrix<double, 4, 4>  matrix_model = model_transform(*m);

        /*  Transform triangles to worldspace. */
        for (const Triangle& t : m->mesh->triangles) {
            triangles.push_back(this->transform_triangle(t, matrix_model));
            active_triangles.push_back(&triangles[triangle_count]);
            triangle_count ++;
        }
    }

    /*  Transform triangles to camera space - TODO. */

    /*  Compute lighting at each vertex (Gouraud Shading) - TODO. */

    /*  Clip against near plane in 3d. */
    this->clip_near_plane(triangles, active_triangles);

    /*  Project triangles into clip space - preserving z coordinate for
        depth comparisons and comparing against the near and far planes. */
    this->perspective_project_triangles(active_triangles);

    /*  Clip triangles - TODO. */
    // this->clip_screen_bounds(triangles, active_triangles);

    /*  Convert triangles to pixel space. */
    // this->convert_triangles_to_pixel_space(active_triangles);

    /*  Raterise triangles. */
    this->rasterise_triangles(active_triangles);
}

double Renderer::get_sign(double val) {
    if (val > 0) {
        return 1.0;
    } else if (val < 0) {
        return -1.0;
    }
    return 0;
};

int Renderer::line_plane_intersection(
    Maths::Vector<double, 4> line_point,
    Maths::Vector<double, 4> line_direction,
    Maths::Vector<double, 4> plane_point,
    Maths::Vector<double, 4> plane_direction_1,
    Maths::Vector<double, 4> plane_direction_2,
    Maths::Vector<double, 4>& out
) {
    /*  Compute d*/
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
    std::list<Triangle*>& active_triangles,
    std::vector<Model*>& models
) {
    for (const Model* m : models) {
        Maths::Matrix<double, 4, 4> model_transform_matrix
            = model_transform(*m);
        
        for (const Triangle& t : m->mesh->triangles) {
            triangles.push_back(transform_triangle(t, model_transform_matrix));
            active_triangles.push_back(&triangles[triangles.size() - 1]);
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

/*  To clip the active triangles against the near plane, we must establish which vertices intersect the plane.*/
void Renderer::clip_near_plane(
    std::vector<Triangle>& triangles,
    std::list<Triangle*>& active_triangles
) {
    int inside_vertices[3];
    int outside_vertices[3];
    int inside_index = 0;
    int outside_index = 0;

    std::list<Triangle*>::iterator itr = active_triangles.begin();

    while (itr != active_triangles.end()) {
        inside_index = 0;
        outside_index = 0;

        Triangle* curr_triangle = *itr;

        /*  Determine inside and outside vertices. */
        for (int i = 0; i < 3; i++) {
            if (curr_triangle->points[i](2) >= this->view_plane_distance) {
                inside_vertices[inside_index] = i;
                inside_index ++;
            } else {
                outside_vertices[outside_index] = i;
                outside_index ++;
            }
        }

        if (outside_index == 3) {
            /*  All vertices are out of bounds - discard. The call to erase
                provides a pointer to the next element, hence we skip the
                increment at the end of the body with a continue. */
            itr = active_triangles.erase(itr);
            continue;
        } else if (outside_index == 2) {
            /*  Two vertices out of bounds - find intersections of their sides
                with the viewing plane. */
            Maths::Vector<double, 4>* inside_vertex =
                &curr_triangle->points[inside_vertices[0]];
            Maths::Vector<double, 4>* outside_vertex_1 =
                &curr_triangle->points[outside_vertices[0]];
            Maths::Vector<double, 4>* outside_vertex_2 =
                &curr_triangle->points[outside_vertices[1]];

            Maths::Vector<double, 4> diff_1 = *outside_vertex_1 -
                *inside_vertex;
            Maths::Vector<double, 4> diff_2 = *outside_vertex_2 -
                *inside_vertex;

            /*  Note that an outside and inside vertex can never have the same
                depth, as they are classified based on their depth. */
            double z_scale_1 =
                ((*outside_vertex_1)(2) - this->view_plane_distance) /
                diff_1(2);
            double z_scale_2 =
                ((*outside_vertex_2)[2] - this->view_plane_distance) /
                diff_2(2);
            
            *outside_vertex_1 = *inside_vertex + z_scale_1 * diff_1;
            *outside_vertex_2 = *inside_vertex + z_scale_2 * diff_2;
        } else if (outside_index == 1) {
            /*  One vertex is out of bounds - find the intersection between
                the lines to the inside vertices and form a quad - then
                decompose into the two triangles. */
            Maths::Vector<double, 4>* inside_vertex_1 =
                &curr_triangle->points[inside_vertices[0]];
            Maths::Vector<double, 4>* inside_vertex_2 =
                &curr_triangle->points[inside_vertices[1]];
            Maths::Vector<double, 4>* outside_vertex =
                &curr_triangle->points[outside_vertices[0]];
            
            Maths::Vector<double, 4> diff_1 = *outside_vertex -
                *inside_vertex_1;
            Maths::Vector<double, 4> diff_2 = *outside_vertex -
                *inside_vertex_2;
            
            double scale_1 = (this->view_plane_distance -
                (*inside_vertex_1)(2)) / diff_1(2);
            double scale_2 = (this->view_plane_distance -
                (*inside_vertex_2)(2)) / diff_2(2);
            
            Maths::Vector<double, 4> new_vertex_1 = (*inside_vertex_1) +
                scale_1 * diff_1;
            Maths::Vector<double, 4> new_vertex_2 = (*inside_vertex_2) +
                scale_2 * diff_2;
            
            /*  Triangle 1 - inside vertices stay the same, outside vertex
                becomes new_vertex_1. This preserves vector windings. */
            curr_triangle->points[outside_vertices[0]] = new_vertex_1;
            
            /*  Triangle 2 - inside vertex 1 becomes new_vertex_1,
                outside_vertex becomes new_vertex_2 and  inside_vertex_2
                stays the same. This preserves the windings. */
            Triangle triangle_2;
            triangle_2.points[inside_vertices[0]] = new_vertex_1;
            triangle_2.points[outside_vertices[0]] = new_vertex_2;
            triangle_2.points[inside_vertices[1]] = *inside_vertex_2;

            triangles.push_back(triangle_2);

            /*  Add to front of linked list to avoid reiterating over clipped
                triangles - will not effect the iterator as std::list has a
                doubly linked list implementation. It is also O(1) because of
                this. */
            active_triangles.push_front(&triangles[triangles.size() - 1]);
        }

        itr ++;
    }
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
    std::list<Triangle*>& active_triangles
) {
    /*  Project all vertices. */
    std::list<Triangle*>::iterator triangle_iterator
        = active_triangles.begin();
    
    while (triangle_iterator != active_triangles.end()) {
        Triangle* curr_tri = *triangle_iterator;

        /*  Project all vertices. */
        for (int i = 0; i < 3; i++) {
            double z_near_div_z = this->view_plane_distance /
                curr_tri->points[i](2);
            curr_tri->points[i](0) *= z_near_div_z;
            curr_tri->points[i](1) *= z_near_div_z;
        }

        triangle_iterator++;
    }
}

void Renderer::rasterise_triangles(std::list<Triangle*>& active_triangles) {

}

}