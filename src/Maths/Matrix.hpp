/*  Matrix.hpp

    A class representing an MxN matrix using template arguments for the element
    type and value arguments for the M and N dimensions. */

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <array>
#include <stdexcept>

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
    
    private:
        static constexpr int num_elems = M * N;
        std::array<T, M * N> data;

        /*  Check indices - for an access to an array, check the first and
            second index fall within [0, M - 1] and [0, N - 1] respectively.
            If the indices are out of bounds, we throw an out_of_range
            exception. */
        void check_indices(size_t index, size_t sub_index) {
            if (index >= M || sub_index >= N) {
                throw std::out_of_range(
                    "Matrix error - attempt to access out-of-range element (" +
                    std::to_string(index) + ", " + std::to_string(sub_index) +
                    ") in matrix of dimensions " + std::to_string(index) + "x"
                    + std::to_string(sub_index) + "."
                );
            }
        }
};

};

#endif