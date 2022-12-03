#include <iostream>
#include <cxxmbd/core.hpp>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        cxxmbd::argument_splitter args { GetCommandLineW() };
        cxxmbd::forward_cl_args(args);
        return 0;
    }
    catch(std::exception& e) {
        std::cout << cxxmbd::ansi::red;
        std::cout << e.what() << "\n\n";
        std::cout << cxxmbd::ansi::reset;
        return -1;
    }
}
