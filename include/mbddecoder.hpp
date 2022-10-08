#ifndef CXXMBD_MBDDECODER_HPP
#define CXXMBD_MBDDECODER_HPP

#include <cstddef>
#include <fstream>
#include "mbdutils.hpp"

namespace cxxmbd {
    template <std::size_t N>
    fs::path
    decode_embed(const binary_embed<N>& file, fs::path dir = fs::current_path()) {
        fs::path opath { file.filename };
        opath = (dir /= opath);
        std::ofstream os (opath, std::ios::out | std::ios::binary );

        if(os.is_open()) {
            os.write(reinterpret_cast<const char*>(file.data), N - 1);
            os.close();
            return opath;
        }
        else throw custom_exception { "Output stream could not be created." };
    }
}

#endif //CXXMBD_MBDDECODER_HPP
