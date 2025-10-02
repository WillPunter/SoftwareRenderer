#include <iostream>

#include "./Maths/Vector.hpp"
#include "./Maths/Matrix.hpp"

/*  Entry point. */
int main() {
    std::cout << "Hello world!" << std::endl;

    Maths::Vector<int, 3> v1 = {1, 2, 3};
    Maths::Vector<int, 3> v2 = {4, 5, 6};

    std::cout << v1 << " + " << v2 << " = " << (v1 + v2) << std::endl;

    Maths::Matrix<int, 3, 4> m1 = {
        1, 2,  3,  4,
        5, 6,  7,  8,
        9, 10, 11, 12
    };

    m1(1, 2) = 100;

    for (int i = 0; i < 3; i++) {
        for(int j = 0; j < 4; j++) {
            std::cout << m1(i, j) << " ";
        }
    }
    std::cout << std::endl;

    std::cout << m1 << std::endl;

    //std::cout << make_one() << std::endl;


    return 0;
}