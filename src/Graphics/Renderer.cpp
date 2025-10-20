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

    /*  Transform triangles to camera space. */
    this->convert_triangles_to_camera_space(
        triangles,
        active_indices,
        scene.camera
    );

    /*  Cull back faces. */
    this->cull_triangle_back_faces(triangles, active_indices);

    /*  Compute lighting at each vertex (Gouraud Shading). */
    this->compute_triangle_lighting(triangles, active_indices, scene.lights);

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
    /*  Copy all points to preserve vertex attributes. */
    Triangle res = triangle;

    for (int i = 0; i < 3; i++) {
        res.points[i].pos = transform * triangle.points[i].pos;
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
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices,
    const Camera& camera
) {
    /*  Build camera transformation - first we translate all the world
        by the reverse of the camera position to move the "camera" to the
        centre of the scene. Then we rotate, also by the reverse of the
        camera, and in the order y-axis, then x-axis, then z-axis. */
    Maths::Matrix<double, 4, 4> camera_transform = Maths::make_rotation_world(
        -camera.rotation(0),
        -camera.rotation(1),
        -camera.rotation(2)
    ) * Maths::make_translation(
        -camera.position(0),
        -camera.position(1),
        -camera.position(2)
    );

    std::list<int>::iterator itr = active_indices.begin();

    while (itr != active_indices.end()) {
        Triangle* curr_triangle = &triangles[*itr];

        *curr_triangle = this->transform_triangle(
            *curr_triangle,
            camera_transform
        );

        itr ++;
    }
}

void Renderer::cull_triangle_back_faces(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    std::list<int>::iterator itr = active_indices.begin();

    Maths::Vector<double, 4> view_dir { 0.0, 0.0, 1.0, 0.0 };

    while (itr != active_indices.end()) {
        Triangle* curr_triangle = &triangles[*itr];

        /*  Determine normal via cross product. Since we are in camera space at
            this stage, if the normal points away from the camera, this is a back
            face and cannot be seen. */
        Maths::Vector<double, 4> side_1 = curr_triangle->points[1].pos -
            curr_triangle->points[0].pos;
        Maths::Vector<double, 4> side_2 = curr_triangle->points[2].pos -
            curr_triangle->points[1].pos;

        Maths::Vector<double, 4> normal = Maths::cross(side_1, side_2);

        if (Maths::dot(normal, view_dir) > 0) {
            itr = active_indices.erase(itr);
        } else {
            itr ++;
        }
    }
}

void Renderer::compute_triangle_lighting(
    std::vector<Triangle>& triangles,
    const std::list<int>& active_indices,
    const std::vector<Light>& lights
) {
    /*  Iterate through active triangles, compute normals and compute
        intensities. */
    std::list<int>::const_iterator itr = active_indices.begin();

    while (itr != active_indices.end()) {
        Triangle* curr_triangle = &triangles[*itr];

        /*  Iterate through lights. */
        for (int i = 0; i < lights.size(); i++) {
            /*  Determine type. */
            if (lights[i].type == Graphics::LightType::AMBIENT) {
                for (int j = 0; j < 3; j++) {
                    curr_triangle->points[j].i += lights[i].intensity;
                }
            } else if (lights[i].type == Graphics::LightType::DIRECTION) {
                /*  Compute normal. */
                Maths::Vector<double, 4> vec_1 = curr_triangle->points[1].pos -
                    curr_triangle->points[0].pos;
                Maths::Vector<double, 4> vec_2 = curr_triangle->points[2].pos -
                    curr_triangle->points[0].pos;
                Maths::Vector<double, 4> normal =
                    Maths::normalise(Maths::cross(vec_1, vec_2));

                /*  Compute dot with light direction. */
                double angle_intensity = Maths::dot(
                    normal,
                    Maths::normalise(lights[i].vec)
                );

                for (int j = 0; j < 3; j++) {
                    curr_triangle->points[j].i += angle_intensity *
                        lights[i].intensity;
                }
            } else if (lights[i].type == Graphics::LightType::POINT) {
                /*  Compute angle with each vertex. */
                for (int j = 0; j < 3; j++) {
                    Maths::Vector<double, 4> direction = Maths::normalise(
                        curr_triangle->points[j].pos - lights[i].vec
                    );

                    double scale = Maths::dot(
                        direction,
                        Maths::normalise(curr_triangle->points[j].pos)
                    );

                    curr_triangle->points[j].i += scale *
                        lights[i].intensity;
                }
            }
        }

        /*  Clamp intensities to range (0, 1). */
        for (int i = 0; i < 3; i++) {
            if (curr_triangle->points[i].i < 0.0) {
                curr_triangle->points[i].i = 0.0;
            } else if (curr_triangle->points[i].i > 1.0) {
                curr_triangle->points[i].i = 1.0;
            }
        }

        itr ++;
    }
}

/*  Make triangles from vertex array - assumes that vertices are in
    an order such that their traversal in order would form a convex
    polygon. */
int Renderer::make_triangles(
    int num_vertices,
    Point in_points[4],
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
        out_triangles[triangle_count].points[0] = in_points[0];
        out_triangles[triangle_count].points[1] = in_points[i];
        out_triangles[triangle_count].points[2] = in_points[i + 1];

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
        [this](Point point) {
            return point.pos(2) >= this->view_plane_distance;
        },

        /*  Lambda for get_intersect operand. This returns a vector storing the
            point where the line vertex_1 -> vertex_2 intersects the
            viewing region boundary. That is a precondition of this function
            and calls on vertex values that do not establish this precondition
            should be considered undefined as this code needs to be called for
            all triangles, and validation per triangle would cost clock
            cycles. */
        [this](
            Point point_1,
            Point point_2
        ) {
            Maths::Vector<double, 4> diff = point_2.pos - point_1.pos;
            double scale = (this->view_plane_distance - point_1.pos(2)) /
                diff(2);

            /*  Since we are in 3d camera space, we also interpolate the depth
                and intensity accordingly. */

            return Point {
                point_1.pos + scale * diff,
                point_1.i + scale * (point_2.i -
                    point_1.i),
                point_1.r + scale * (point_2.r - point_1.r),
                point_1.g + scale * (point_2.g - point_1.g),
                point_1.b + scale * (point_2.b - point_1.b),
                point_1.tex_x + scale * (point_2.tex_x - point_1.tex_x),
                point_1.tex_y + scale * (point_2.tex_y - point_1.tex_y),
                0.0, 0.0,
                0.0, 0.0, 0.0,
                0.0, 0.0
            };
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
                curr_tri->points[i].pos(2);
            curr_tri->points[i].pos(0) *= z_near_div_z;
            curr_tri->points[i].pos(1) *= z_near_div_z;

            /*  Since we now convert to 2d space, the vertex attributes no
                longer vary linearly with the new screen space coordinates.
                Therefore, we have to interpolate against each attribute / z
                instead, so we store these in the points.
                
                Note that since we have already clipped against the near plane,
                we can assume that the z coordinate is > 0. */
            curr_tri->points[i].inv_z = 1.0 / curr_tri->points[i].pos(2);
            curr_tri->points[i].i_div_z = curr_tri->points[i].i /
                curr_tri->points[i].pos(2);
            curr_tri->points[i].r_div_z = curr_tri->points[i].r /
                curr_tri->points[i].pos(2);
            curr_tri->points[i].g_div_z = curr_tri->points[i].g /
                curr_tri->points[i].pos(2);
            curr_tri->points[i].b_div_z = curr_tri->points[i].b /
                curr_tri->points[i].pos(2);
            curr_tri->points[i].tex_x_div_z = curr_tri->points[i].tex_x /
                curr_tri->points[i].pos(2);
            curr_tri->points[i].tex_y_div_z = curr_tri->points[i].tex_y /
                curr_tri->points[i].pos(2);
        }

        itr++;
    }
}

/*  Helper function for clipping against the bounds of the 2d viewing plane. */
static Point make_scaled_point_2d(
    Point point_1,
    Point point_2,
    double scale,
    Maths::Vector<double, 4> diff
) {
    return Point {
        point_1.pos + scale * diff,
        
        /*  Intensity, red, green, blue, tex_x, tex_y - we used the
            a / z variants below now as we are in 2d perspective
            projected space so these attributes no longer vary linearly
            with the coordinates in pos. */
        0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0,
        
        /*  Attributes / z: 1 / z, intensity, red, green, blue, texture
            x, texture y. */
        point_1.inv_z + scale * (point_2.inv_z - point_1.inv_z),
        point_1.i_div_z + scale * (point_2.i_div_z - point_1.i_div_z),
        point_1.r_div_z + scale * (point_2.r_div_z - point_1.r_div_z),
        point_1.g_div_z + scale * (point_2.g_div_z - point_1.g_div_z),
        point_1.b_div_z + scale * (point_2.b_div_z - point_1.b_div_z),
        point_1.tex_x_div_z + scale * (point_2.tex_x_div_z -
            point_1.tex_x_div_z),
        point_1.tex_y_div_z + scale * (point_2.tex_y_div_z -
            point_1.tex_y_div_z)
    };
}

/*  Clip in 2d against the left bound of the screen. */
void Renderer::clip_left_bound(
    std::vector<Triangle>& triangles,
    std::list<int>& active_indices
) {
    this->clip_triangles(
        triangles,
        active_indices,

        [this](Point& point) {
            return point.pos(0) > this->screen_left_bound;
        },

        [this](
            Point& point_1,
            Point& point_2
        ) {
            Maths::Vector<double, 4> diff = point_2.pos - point_1.pos;
            double scale = (this->screen_left_bound - point_1.pos(0)) / diff(0);
            return make_scaled_point_2d(point_1, point_2, scale, diff);
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

        [this](Point& point) {
            return point.pos(0) < this->screen_right_bound;
        },

        [this](
            Point& point_1,
            Point& point_2
        ) {
            Maths::Vector<double, 4> diff = point_2.pos - point_1.pos;
            double scale = (this->screen_right_bound - point_1.pos(0)) /
                diff(0);
            return make_scaled_point_2d(point_1, point_2, scale, diff);
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

        [this](Point& point) {
            return point.pos(1) < this->screen_top_bound;
        },

        [this](
            Point& point_1,
            Point& point_2
        ) {
            Maths::Vector<double, 4> diff = point_2.pos - point_1.pos;
            double scale = (this->screen_top_bound - point_1.pos(1)) / diff(1);
            return make_scaled_point_2d(point_1, point_2, scale, diff);
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

        [this](Point& point) {
            return point.pos(1) > this->screen_bottom_bound;
        },

        [this](
            Point& point_1,
            Point& point_2
        ) {
            Maths::Vector<double, 4> diff = point_2.pos - point_1.pos;
            double scale = (this->screen_bottom_bound - point_1.pos(1)) /
                diff(1);
            return make_scaled_point_2d(point_1, point_2, scale, diff);
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
            curr_triangle->points[i].pos(0) = round(
                ((curr_triangle->points[i].pos(0) - this->screen_left_bound) /
                (this->screen_right_bound - this->screen_left_bound)) *
                (buffer_width - 1)
            );
            
            curr_triangle->points[i].pos(1) = (buffer_height - 1) - round(
                ((curr_triangle->points[i].pos(1) - this->screen_bottom_bound) /
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

        draw_shaded_triangle(
            render_window,

            {
                static_cast<int>(floor(curr_triangle->points[0].pos(0))),
                static_cast<int>(floor(curr_triangle->points[0].pos(1))),
                curr_triangle->points[0].inv_z,
                curr_triangle->points[0].i_div_z,
                curr_triangle->points[0].r_div_z,
                curr_triangle->points[0].g_div_z,
                curr_triangle->points[0].b_div_z,
                curr_triangle->points[0].tex_x_div_z,
                curr_triangle->points[0].tex_y_div_z
            },

            {
                static_cast<int>(floor(curr_triangle->points[1].pos(0))),
                static_cast<int>(floor(curr_triangle->points[1].pos(1))),
                curr_triangle->points[1].inv_z,
                curr_triangle->points[1].i_div_z,
                curr_triangle->points[1].r_div_z,
                curr_triangle->points[1].g_div_z,
                curr_triangle->points[1].b_div_z,
                curr_triangle->points[1].tex_x_div_z,
                curr_triangle->points[1].tex_y_div_z
            },

            {
                static_cast<int>(floor(curr_triangle->points[2].pos(0))),
                static_cast<int>(floor(curr_triangle->points[2].pos(1))),
                curr_triangle->points[2].inv_z,
                curr_triangle->points[2].i_div_z,
                curr_triangle->points[2].r_div_z,
                curr_triangle->points[2].g_div_z,
                curr_triangle->points[2].b_div_z,
                curr_triangle->points[2].tex_x_div_z,
                curr_triangle->points[2].tex_y_div_z
            }
        );
        
        itr ++;
    }
}

}