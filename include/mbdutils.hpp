#ifndef CXXMBD_MBDUTILS_HPP
#define CXXMBD_MBDUTILS_HPP

#include <exception>
#include <filesystem>
#include <string>

#define EMBED_POINT

template <std::size_t N>
using byte_block_t = unsigned char[N];

template <std::size_t N>
struct binary_embed {
    std::string filename;
    byte_block_t<N> data;
};

namespace cxxmbd {
    namespace fs = std::filesystem;
    using byte_t = unsigned char;

    struct custom_exception : std::exception {
        custom_exception(const std::string&& str) : data(str) {}

        [[nodiscard]]
        const char*
        what() const noexcept {
            return data.c_str();
        }
    private:
        const std::string data;
    };
}

#endif //CXXMBD_MBDUTILS_HPP
