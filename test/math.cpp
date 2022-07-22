#include <xbond/math/rand.hpp>
#include <iostream>

int math_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::cout << "\trand: \n\t\t";
    for (int i=0;i<10;++i) std::cout << xbond::math::rand::integer(10, 80) << " ";
    std::cout << "\n\t\t";
    for (int i=0;i<10;++i) std::cout << xbond::math::rand::integer(100, 800) << " ";
    std::cout << "\n";
    return 0;
}