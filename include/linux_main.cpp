#include "cxxmbd.hpp"

int main(int argc, char* argv[]) {
    try {
        cxxmbd::handle_cl_args(argc, argv);
    }
    catch(std::exception& e) {
        std::cout << e.what() << "\n\n";
    }
}
