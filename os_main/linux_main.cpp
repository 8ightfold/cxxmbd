#include <cxxmbd/cxxmbd.hpp>

int main(int argc, char* argv[]) {
    try {
        cxxmbd::handle_cl_args(argc, argv);
    }
    catch(std::exception& e) {
        std::cerr << e.what() << "\n\n";
    }
}
