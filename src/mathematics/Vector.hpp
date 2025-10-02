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

template <typename T, unsigned int N>
class Vector {
    public:
        /*  Default constructor - set elements to zero. */
        Vector() {};
    
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

        /*  Subscript operator.
        
            Provides array-style access to elements. We return a reference so
            that we can read from or write to the element with the same
            definition. */
        T& operator[](int index) {
            if (index < 0 || index >= N) {
                throw std::out_of_range(
                    "Vector error - attempt to access element " +
                    std::to_string(index) + " in Vector of dimension " +
                    std::to_string(N) + "."
                );
            }

            return data[index];
        }

        /*  To string.
        
            We print vectors in row form, as:
                (v1, v2, ..., vn)^T
            This is subject to change depending on how nice it looks in
            practice. */
        std::string to_string() const {
            std::string res = "(";

            for (int i = 0; i < N; i++) {
                res += std::to_string(data[i]);

                if (i != N - 1) {
                    res += ", ";
                }
            }

            res += ")^T";

            return res;
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

        /*  Binary addition - note that we take the arguments by reference to
            avoid copying data, and return by value since we must create a new
            vector to return. */
        friend Vector<T, N> operator+(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
            Vector<T, N> res;

            for (int i = 0; i < N; i++) {
                res.data[i] = lhs.data[i] + rhs.data[i];
            }


            return res;
        };
        
    private:
        std::array<T, N> data;
};

#endif