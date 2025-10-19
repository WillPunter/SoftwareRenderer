/*  Model.hpp

    This file provides the model structure as well as the supporting data
    structures. */
#ifndef MODEL_HPP
#define MODEL_HPP

#include "./../Maths/Matrix.hpp"
#include "./../Maths/Vector.hpp"
#include <vector>

namespace Graphics {

struct Triangle {
    Maths::Vector<double, 4> points[3];
    double intensities[3];
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