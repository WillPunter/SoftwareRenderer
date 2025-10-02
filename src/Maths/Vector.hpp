/*  Vector.hpp

    A generic vector class template that supports the typical operations.
    
    It is generic in the dimensions (N) and the element type (T).
    
    The element type must support + and * operations. NOTE THAT THIS IS NOT
    CURRENTLY CAUGHT BY THE RELEVANT MECHANISMS SO WATCH OUT FOR UNDEFINED
    ERRORS.
    
    The vector data is stored as a static array. The majority of vectors used
    in 3d graphics will be 3 or 4 dimensions, and heap allocations are less
    performant (copies and allocations are likely common in graphics code). */

#ifndef VECTOR_HPP
#define VECTOR_HPP

#include<array>
#include<stdexcept>
#include<ostream>

namespace Maths {

template <typename T, unsigned int N>
class Vector {
    public:
        /*  Default constructor - set elements to zero. */
        Vector() {
            for (int i = 0; i < N; i++) {
                this->data[i] = 0;
            }
        };
    
        /*  Initializer constructor.
        
            If K elements are provided, and K <= N, the first K elements in the
            vector are set to the corresponding values in the initializer list
            and then the rest are set to 0.
            
            If the initializer list exceeds N then we throw a length exception. */
        Vector(std::initializer_list<T> elems) {
            int i {0};

            if (elems.size() > N) {
                throw std::length_error(
                    "Vector error - cannot assign initializer_list of length "
                    + std::to_string(elems.size()) + " to Vector of dimension "
                    + std::to_string(N) + "."
                );
            }

            for (const T& elem : elems) {
                data[i] = elem;
                i++;
            }

            for (; i < N; i++) {
                data[i] = 0;
            }
        }

        /*  Call / access operator.
        
            Provides individual access to elements. We return a reference so
            that we can read from or write to the element with the same
            definition.
            
            An out_of_range exception will be thrown by the check_index call if
            index is outside of the range [0, N - 1].
            
            NOTE: we do not use the conventional subscript operator. The reason
            for this is because syntactic similarity between element accesses
            for vectors and matrices is desirable (since each row of a matrix
            can itself be thought of as a vector). The subscript operator ([])
            only supports a single argument. Therefore, implementing matrix
            element accesses would require use of [][] syntax, which would
            require a proxy object to be returned. This is problematic as this
            type is not really part of the mathematical usage of a matrix, but
            if made private, can still be bypassed using auto. () offers simple
            and equally readable syntax (if a little different to other
            standards) that is consistent for vector and matrix accesses, and
            for the latter produces no object creation / management overhead or
            any of the associated issues. */
        T& operator()(size_t index) {
            this->check_index(index);
            return data[index];
        }

        /*  Subscript operator for const accesses.
        
            We still want to support accesses for const Vectors via the
            subscript operator. To do this, we require a const reference to be
            returned and the member function to be marked as const (so as to
            indicate that it does not modify the object).
            
            An out_of_range exc{eption will be thrown by the check_index call if
            index is outside of the range [0, N]. */
        const T& operator()(int index) const {
            this->check_index(index);
            return data[index];
        };

        /*  Operation-assignment operators - +=, -=. These have the expected
            implementations. */
        Vector<T, N>& operator+=(Vector<T, N>& other) {
            for (int i = 0; i < N; i++) {
                this->data[i] += other.data[i];
            }

            return *this;
        }

        Vector<T, N>& operator-=(Vector<T, N>& other) {
            for (int i = 0; i < N; i++) {
                this->data[i] -= other.data[i];
            }

            return *this;
        }

        /*  Friend functions:
                We define binary operations (+, -, *, etc.) on vectors as
                non-member functions so as to avoid giving special treatment to
                either operation.
                
                These are friend to allow for direct access to their data,
                bypassing additional function calls. 
                
                We define friend functions in-place due to the use of the T and
                N template parameters, and the clutter required to add them to
                a different file. */
        
        /*  Binary addition - we take arguments as const references. This
            prevents copies being made whilst also allowing temporaries to be
            used in an intuitive manner. */
        friend Vector<T, N>
        operator+(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
            return zip_elems(lhs, rhs, [](T x, T y){return x + y; });
        }

        /*  Binary subtraction - we take arguments by reference and return by
            value for the same reason as binary addition. */
        friend Vector<T, N>
        operator-(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
            return zip_elems(lhs, rhs, [](T x, T y){ return x - y; });
        }

        /*  Scalar multiplication - we apply the * operation to all elements
            to define the mathematical scalar multiplication. */
        friend Vector<T, N>
        operator*(const Vector<T, N>& vec, T scalar) {
            return vec.map_elems([scalar](T elem) { return scalar * elem; });
        }

        /*  Mathematical scalar multiplication is commutative so we implement
            in terms of vector * scalar multiplication (see above for
            details). */
        friend Vector<T, N>
        operator*(T scalar, const Vector<T, N>& vec) {
            return vec * scalar;
        }

        /*  Scalar / dot product. Note that we default-initalise an accumulator
            of type T, so the type used must support this in the expected
            manner. */
        friend Vector<T, N> dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
            T sum {};

            for (int i = 0; i < N; i++) {
                sum += lhs.data[i] * rhs.data[i];
            }

            return sum;
        }

        /*  Cross product - might be best implemented in terms of matrix
            determinant? TODO. */

        /*  Inserting into a stream - we overload the << operator for output
            streams to allow the use of cout and similar. We insert a string
            of the form:
                (v1, v2, ..., vN)^T
            as this is a typical vector representation in text (it is a row
            vector transposed to produce a column vector). */
        friend std::ostream& operator<<(std::ostream& output_stream, Vector<T, N> vec) {
            output_stream << "(";

            for (int i = 0; i < N; i++) {
                output_stream << vec.data[i];

                if (i < N - 1) {
                    output_stream << ", ";
                }
            }

            output_stream << ")^T";

            return output_stream;
        }

    private:
        std::array<T, N> data;

        /*  Check index - throws an exception if the provided index is outside
            of the range [0, N]. Otherwise it does not do anything. */
        void check_index(size_t index) {
            if (index >= N) {
                throw std::out_of_range(
                    "Vector error - attempt to access element " +
                    std::to_string(index) + " in Vector of dimension " +
                    std::to_string(N) + "."
                );
            }
        }

        /*  Unary element-wise operation - this member function takes a functor
            to be applied to each element (should take a T and produce a T). */
        template <typename F>
        Vector<T, N> map_elems(F op) const {
            Vector<T, N> res;

            for (int i = 0; i < N; i++) {
                res.data[i] = op(this->data[i]);
            }

            return res;
        }

        /*  Binary Element-wise operation - this function is a generic
            algorithm for doing something on two vectors to produce a new,
            third vector on an element-by-element basis (e.g. vector addition,
            vector subtraction, etc.).
            
            It is implemented as a private static member function because it
            should not be part of the public interface (which should only
            consist of the expected mathematical operations and I/O
            functionality). All functions that call this are friend functions
            implementing binary operators (see above). */
        template <typename F>
        static Vector<T, N>
        zip_elems(const Vector<T, N>& v1, const Vector<T, N>& v2, F op) {
            Vector<T, N> res;

            for (int i = 0; i < N; i++) {
                res.data[i] = op(v1.data[i], v2.data[i]);
            }

            return res;
        }
};

};

#endif