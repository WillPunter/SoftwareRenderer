#include <iostream>

#include "./mathematics/Vector.hpp"

/*  Entry point. */
int main() {
    std::cout << "Hello world!" << std::endl;

    Vector<int, 3> v1 = {1, 2, 3};
    Vector<int, 3> v2 = {4, 5, 6};

    std::cout << v1 << " + " << v2 << " = " << (v1 + v2) << std::endl;

    //std::cout << make_one() << std::endl;


    return 0;
}