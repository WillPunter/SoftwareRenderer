/*  Transform.cpp

    Implementations of transformation matrices and their associated
    operations. */

#include "Transform.hpp"

#include <cmath>

namespace Maths {

Matrix<double, 4, 4> make_enlargement(double x, double y, double z) {
    return Matrix<double, 4, 4> {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };
}

Matrix<double, 4, 4> make_rotation_yz_plane(double x) {
    double sin_x = sin(x);
    double cos_x = cos(x);

    return Matrix<double, 4, 4> {
        1, 0,      0,     0,
        0, cos_x,  sin_x, 0,
        0, -sin_x, cos_x, 0,
        0, 0,      0,     1
    };
}

Matrix<double, 4, 4> make_rotation_xz_plane(double y) {
    double sin_y = sin(y);
    double cos_y = cos(y);

    return Matrix<double, 4, 4> {
        cos_y, 0, -sin_y, 0,
        0, 1,  0,         0,
        sin_y, 0, cos_y,  0,
        0,     0, 0,      1
    };
}

Matrix<double, 4, 4> make_rotation_xy_plane(double z) {
    double sin_z = sin(z);
    double cos_z = cos(z);

    return Matrix<double, 4, 4> {
        cos_z, -sin_z, 0, 0,
        sin_z,  cos_z, 0, 0,
        0,      0,     1, 0,
        0,      0,     0, 1
    };
}

Matrix<double, 4, 4> make_rotation_model(double x, double y, double z) {
    return make_rotation_xz_plane(y) * make_rotation_xy_plane(z) *
        make_rotation_yz_plane(x);
}

Matrix<double, 4, 4> make_rotation_world(double x, double y, double z) {
    return make_rotation_yz_plane(x) * make_rotation_xz_plane(y) *
        make_rotation_xy_plane(z);
}

Matrix<double, 4, 4> make_translation(double x, double y, double z) {
    return Matrix<double, 4, 4> {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    };
}

Matrix<double, 4, 4> make_homogeneous_projection(double plane_distance) {
    return Matrix<double, 4, 4> {
        plane_distance, 0, 0, 0,
        0, plane_distance, 0, 0,
        0, 0, plane_distance, 0,
        0, 0, 1, 0
    };
}

}