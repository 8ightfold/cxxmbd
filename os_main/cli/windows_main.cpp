#include <cxxmbd/cxxmbd.hpp>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        cxxmbd::argument_splitter args { GetCommandLineW() };
        cxxmbd::handle_cl_args(args);
    }
    catch(std::exception& e) {
        std::cout << e.what() << "\n\n";
    }
    return 0;
}
