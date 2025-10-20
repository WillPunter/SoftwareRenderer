/*  Model.hpp

    This file provides the model structure as well as the supporting data
    structures. */
#ifndef MODEL_HPP
#define MODEL_HPP

#include "./../Maths/Matrix.hpp"
#include "./../Maths/Vector.hpp"
#include <vector>

namespace Graphics {

struct Point {
    Maths::Vector<double, 4> pos;

    /*  Vertex attributes - for use in camera space: intensity, red, green,
        blue, texture x, texture y. */
    double i;
    double r;
    double g;
    double b;
    double tex_x;
    double tex_y;

    /*  Vertex attributes divided by z - for use in 2d screen space. */
    double inv_z;
    double i_div_z;
    double r_div_z;
    double g_div_z;
    double b_div_z;
    double tex_x_div_z;
    double tex_y_div_z;
};

struct Triangle {
    Point points[3];
};

struct Mesh {
    std::vector<Triangle> triangles;
};

struct Model {
    Mesh* mesh;

    /*  Position in homogeneous coordinates: (x, y, z, w). */
    Maths::Vector<double, 4> position;

    /*  Scale in homogeneous coordinates: (x, y, z, w). */
    Maths::Vector<double, 4> scale;

    /*  Rotation in homogeneous coordinates - Euler angles / roll-pitch-yaw.
        (x, y, z, w) where:
            x is the angle in the y-z plane.
            y is the angle in the x-z plane.
            z is the angle in the x-y plane. */
    Maths::Vector<double, 4> rotation;
};

/*  To transform a model into world space, first scale, then rotate and then
    translate. */
Maths::Matrix<double, 4, 4> model_transform(const Model& model);

}

#endif