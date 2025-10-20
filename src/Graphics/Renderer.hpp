/*  Renderer.hpp

    Stores renderer state (e.g. FOV, aspect ratio, etc.) and performs
    perspective projections using it.
    
    The Renderer is responsible for 3d graphics manipulation. It is not
    responsible for 2d primitive rendering - see the Rasteriser for this. */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "./../Maths/Vector.hpp"
#include "./../System/RenderWindow.hpp"
#include "Model.hpp"
#include "./../Maths/Transform.hpp"

#include <list>
#include <vector>

namespace Graphics {

struct Camera {
    Maths::Vector<double, 4> position;
    Maths::Vector<double, 4> rotation;
};

enum class LightType {
    AMBIENT,
    DIRECTION,
    POINT
};

struct Light {
    LightType type;
    double intensity;
    Maths::Vector<double, 4> vec;
};

struct Scene {
    std::vector<Model*> models;
    std::vector<Light> lights;
    Camera camera;
};

class Renderer {
    public:
        Renderer(double fov, double aspect_ratio, double far_plane_distance);

        void render_scene(
            System::RenderWindow& render_window,
            const Scene& scene
        );

    private:     
        Triangle transform_triangle(
            const Triangle& triangle,
            const Maths::Matrix<double, 4, 4>& transform
        );

        void build_triangles_list_from_models(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices,
            std::vector<Model*>& models
        );

        void convert_triangles_to_camera_space(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices,
            const Camera& camera
        );

        void cull_triangle_back_faces(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        /*  Compute triangle lighting in scene. */
        void compute_triangle_lighting(
            std::vector<Triangle>& triangles,
            const std::list<int>& active_indices,
            const std::vector<Light>& lights
        );

        /*  Clip vertices - this clips the vertices by walking through each line
            segment of a triangle. The resulting array of vertices, out, will either
            contain 0 vertices (the entire triangle is outside of the viewing region),
            3 vertices (one triangle in the plane can be made) or 4 (a quad in the
            viewing region can be made).

            Note that this is defined in the header as it is a template function.

            This algorithm works by considering each connected pair of nodes in the
            triangle (i, j) - in the code these are represented by the vertex array
            order, e.g. for { v1, v2, v3 } we would consider these sides to be the
            lines (v1, v2) (from v1 to v2), (v2, v3) (from v2 to v3), (v3, v1) (from v3
            to v1). We keep a sequence of vertices to output called out:
                if i is in the viewing region, we add i to out.

                if the line i -> j crosses the viewing region boundary, we add the
                intersection of the line with the boundary to out.

            The intuition behind this is that we walk from each vertex to it's
            subsequent vertex according to the original winding (i.e. the order of the
            vertices). When we cross an intersection between the viewing region and
            outside, we add that vertex when we would encounter it on our walk. Since
            we do not add vertices outside of the the viewing region, we automatically
            connect adjacent intersections where a line exists completely out of the
            plane - connecting the last intersection before we "left" and the first
            intersection when we "returned".
            
            Understand that the result of this function is simply a shape (a triangle,
            a quad or no shape at all). The windings / direction of the lines around
            the perimeter are consistent with one another (as in they connect to form a
            clockwise or anticlockwise traversal of the shape - they all follow tip to
            tail) and are preserved (i.e. have the same direction / ordering as the
            original shape) because the points added are added in order of their
            traversal around the perimeter according to the same winding. */
        template <typename F, typename G>
        int clip_points(
            Triangle* triangle,
            Point out[4],
            F in_viewing_region,
            G get_intersect
        ) {
            bool in_points[3] = {
                in_viewing_region(triangle->points[0]),
                in_viewing_region(triangle->points[1]),
                in_viewing_region(triangle->points[2])
            };

            int out_index = 0;
            int next_i = 0;

            for (int i = 0; i < 3; i++) {
                if (in_points[i]) {
                    out[out_index] = triangle->points[i];
                    out_index ++;
                }

                next_i = (i + 1) % 3;

                if (in_points[i] != in_points[next_i]) {
                    /*  Line vertex[i] -> vertex[next_i] cross viewing region -
                        find intersection and add it as next point. */
                    out[out_index] = get_intersect(
                        triangle->points[i],
                        triangle->points[next_i]
                    );
                    out_index ++;
                }
            }

            return out_index;
        }

        int make_triangles(
            int num_vertices,
            Point in_points[4],
            Triangle out_triangles[2]
        );

        /*  Clip triangles - another template function, this time that clips
            a list of triangles using specific region-checking and
            interpolation functors. Once again, this must be defined in the
            class declaration as it is a template function. */
        template <typename F, typename G>
        void clip_triangles(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices,
            F in_viewing_region,
            G get_intersect
        ) {
            std::list<int>::iterator itr = active_indices.begin();
            Point clipped_points[4];
            int num_clipped_points = 0;
            Triangle out_triangles[2];
            int num_triangles = 0;

            while (itr != active_indices.end()) {
                Triangle* curr_triangle = &triangles[*itr];

                /*  Clip all vertices to obtain nothing, a triangle or a
                    quad. */
                num_clipped_points = clip_points(
                    curr_triangle,
                    clipped_points,
                    in_viewing_region,
                    get_intersect
                );

                /*  Partition the given shape into triangles. */
                num_triangles = make_triangles(
                    num_clipped_points,
                    clipped_points,
                    out_triangles
                );

                /*  Add corresponding triangles to passed structures. */
                if (num_triangles == 0) {
                    /*  Remove current triangle - fully clipped. Note that the
                        erase member function on std::list returns the next
                        iterator, so we can skip the increment below. */
                    itr = active_indices.erase(itr);
                    continue;
                } else if (num_triangles == 1) {
                    /*  Copy resulting triangle to current triangle address. */
                    *curr_triangle = out_triangles[0];
                } else if (num_triangles == 2) {
                    /*  Copy resulting triangle and add new triangle. */
                    *curr_triangle = out_triangles[0];
                    triangles.push_back(out_triangles[1]);
                    active_indices.push_front(
                        triangles.size() - 1
                    );
                }

                itr ++;
            }
        }

        void clip_near_plane(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void perspective_project_triangles(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void clip_left_bound(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void clip_right_bound(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void clip_top_bound(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void clip_bottom_bound(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void clip_screen_bounds(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        void convert_triangles_to_pixel_space(
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices,
            int buffer_width,
            int buffer_height
        );
    
        void rasterise_triangles(
            System::RenderWindow& render_window,
            std::vector<Triangle>& triangles,
            std::list<int>& active_indices
        );

        double fov;
        double aspect_ratio;
        double view_plane_distance;
        double far_plane_distance;

        const double screen_left_bound;
        const double screen_right_bound;
        const double screen_top_bound;
        const double screen_bottom_bound;
        
};

}

#endif