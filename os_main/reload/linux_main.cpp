#include <iostream>
#include <cxxmbd/core.hpp>

int main(int argc, char* argv[]) {
    try {
        cxxmbd::forward_cl_args(argc, argv);
        return 0;
    }
    catch(std::exception& e) {
        std::cout << cxxmbd::ansi::red;
        std::cout << e.what() << "\n\n";
        std::cout << cxxmbd::ansi::reset;
        return -1;
    }
}
