/*  Model.hpp

    This file provides the model structure as well as the supporting data
    structures. */
#ifndef MODEL_HPP
#define MODEL_HPP

#include "./../Maths/Vector.hpp"
#include <vector>

struct Triangle {
    Maths::Vector<double, 4> points[3];
};

struct Mesh {
    std::vector<Triangle> triangles;
};

struct Model {
    Mesh* mesh;
    Maths::Vector<double, 4> position;
    Maths::Vector<double, 4> scale;
    Maths::Vector<double, 4> rotation;
};

#endif