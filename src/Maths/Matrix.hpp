/*  Matrix.hpp

    A class representing an MxN matrix using template arguments for the element
    type and value arguments for the M and N dimensions. */

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <array>
#include <stdexcept>
#include <ostream>

#include "Vector.hpp"

namespace Maths {

template <typename T, unsigned int M, unsigned int N>
class Matrix {
    public:
        /*  Default constructor - set all elements to zero. */
        Matrix() {
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    this->data[i * N + j] = 0;
                }
            }
        }

        /*  Initializer list constructor - fill in the elements. A length_error
            exception is thrown on an attempt to assign more elements than
            permitted to the array. */
        Matrix(std::initializer_list<T> elems) {
            if (elems.size() > M * N) {
                throw std::length_error(
                    "Matrix error - cannot assign initializer_list of size " +
                    std::to_string(elems.size()) + " to matrix of dimensions "
                    + std::to_string(M) + "x" + std::to_string(N) + " (" +
                    std::to_string(num_elems) + " elements)."
                );
            }

            int i {0};

            for (const T& elem : elems) {
                this->data[i] = elem;
                i++;
            }
        }

        /*  Call / access operator - for array access. */
        T& operator()(size_t index, size_t sub_index) {
            this->check_indices(index, sub_index);
            return this->data[index * N + sub_index];
        }

        /*  Call / access operator for const accesses. */
        const T& operator()(size_t index, size_t sub_index) const {
            this->check_indices(index, sub_index);
            return this->data[index * N + sub_index];
        }

        /*  Unary negation operator. */
        Matrix<T, M, N> operator-() {
            return this->map_elems([](T x) { return -x; });
        }

        /*  Assignment operators. */
        Matrix<T, M, N>& operator+=(const Matrix<T, M, N>& other) {
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    this->data[i * N + j] += other(i, j);
                }
            }

            return *this;
        }

        Matrix<T, M, N>& operator-=(const Matrix<T, M, N>& other) {
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    this->data[i * N + j] -= other(i, j);
                }
            }

            return *this;
        }

        Matrix<T, M, N>& operator*=(T scalar) {
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    this->data[i * N + j] *= scalar;
                }
            }

            return *this;
        }

        /*  Friend functions. */
        friend Matrix<T, M, N> operator+(
            const Matrix<T, M, N>& lhs,
            const Matrix<T, M, N>& rhs
        ) {
            return zip_elems(lhs, rhs, [](T x, T y) { return x + y; });
        }

        friend Matrix<T, M, N> operator-(
            const Matrix<T, M, N>&  lhs,
            const Matrix<T, M, N>& rhs
        ) {
            return zip_elems(lhs, rhs, [](T x, T y) { return x - y; });
        }

        friend Matrix<T, M, N> operator*(
            const Matrix<T, M, N>& mat,
            T scalar
        ) {
            return mat.map_elems([scalar](T x){ return scalar * x; });
        }

        friend Matrix<T, M, N> operator*(
            T scalar,
            const Matrix<T, M, N>& mat
        ) {
            return mat * scalar;
        }
    
    private:
        static constexpr int num_elems = M * N;
        std::array<T, M * N> data;

        /*  Check indices - for an access to an array, check the first and
            second index fall within [0, M - 1] and [0, N - 1] respectively.
            If the indices are out of bounds, we throw an out_of_range
            exception. */
        void check_indices(size_t index, size_t sub_index) const {
            if (index >= M || sub_index >= N) {
                throw std::out_of_range(
                    "Matrix error - attempt to access out-of-range element (" +
                    std::to_string(index) + ", " + std::to_string(sub_index) +
                    ") in matrix of dimensions " + std::to_string(index) + "x"
                    + std::to_string(sub_index) + "."
                );
            }
        }

        /*  Element-wise operation - perform an operation on every pair of
            elements for two matrices (lhs and rhs). This takes a functor
            to be called on all pairs of elements. */
        template<typename F>
        static Matrix<T, M, N> zip_elems(
            const Matrix<T, M, N>& lhs,
            const Matrix<T, M, N>& rhs,
            F op
        ) {
            Matrix<T, M, N> res;

            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    res(i, j) = op(lhs(i, j), rhs(i, j));
                }
            }

            return res;
        }

        /*  Map elements. */
        template<typename F>
        Matrix<T, M, N> map_elems(
            F op
        ) {
            Matrix<T, M, N> res;

            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    res(i, j) = op(this->data[i * N + j]);
                }
            }

            return res;
        }
};

/*  Send matrix to output stream. */
template <typename T, unsigned int M, unsigned int N>
std::ostream& operator<<(
    std::ostream& output_stream,
    const Matrix<T, M, N>& mat
) {
    output_stream << "[";

    for (int i = 0; i < M; i++) {
        output_stream << "[";

        for (int j = 0; j < N; j++) {
            output_stream << mat(i, j);

            if (j < N - 1) {
                output_stream << ", ";
            }
        }

        output_stream << "]";

        if (i < M - 1) {
            output_stream << ", ";
        }
    }

    output_stream << "]";

    return output_stream;
}

/*  Matrix-vector multiplication - mathematically we define this for an MxN
    matrix with an N dimensional vector to produce a vector of dimension M. */
template<typename T, unsigned int M, unsigned int N>
Vector<T, M> operator*(const Matrix<T, M, N>& mat, const Vector<T, N>& vec) {
    Vector<T, M> res;

    for (int i = 0; i < M; i++) {
        T sum {};

        for (int j = 0; j < N; j++) {
            sum += mat(i, j) * vec(j);
        }

        res(i) = sum;
    }

    return res;
}

/*  Matrix-matrix multiplication - mathematically this is defined for any MxK
    matrix and a KxN matrix. */
template<typename T, unsigned M, unsigned K, unsigned N>
Matrix<T, M, N> operator*(
    const Matrix<T, M, K>& lhs,
    const Matrix<T, K, N>& rhs
) {
    Matrix<T, M, N> res;

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            T sum {};

            for (int k = 0; k < K; k++) {
                sum += lhs(i, k) * rhs(k, j);
            }

            res(i, j) = sum;
        }
    }

    return res;
}

};

#endif