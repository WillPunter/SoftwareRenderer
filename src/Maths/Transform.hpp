/*  Transform.hpp

    A collection of functions for implementing geometric transformations for 3d
    graphics. */

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Matrix.hpp"
#include "Vector.hpp"

namespace Maths {

/*  Construct NxN identity matrix. */
template<unsigned int N>
Matrix<double, N, N> make_identity() {
    Matrix<double, N, N> res;

    for (int i = 0; i < N; i++) {
        res(i, i) = 1;
        res(i, N - 1) = 1;
    }

    return res;
}

/*  3d Enlargement matrix using homogeneous coordinates. */
Matrix<double, 4, 4> make_enlargement(double x, double y, double z);

/*  Rotation matrices using homogeneous coordinates. */
Matrix<double, 4, 4> make_rotation_yz_plane(double x);
Matrix<double, 4, 4> make_rotation_xz_plane(double y);
Matrix<double, 4, 4> make_rotation_xy_plane(double z);

/*  Object rotation - rotate in yz, then xy, and finally xz. */
Matrix<double, 4, 4> make_rotation_model(double x, double y, double z);

/*  World rotation - rotate in xy, then xz then yz. */
Matrix<double, 4, 4> make_rotation_world(double x, double y, double z);

/*  Translation matrix. */
Matrix<double, 4, 4> make_translation(double x, double y, double z);

}

#endif