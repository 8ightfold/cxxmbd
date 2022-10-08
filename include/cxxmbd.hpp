#ifndef CXXMBD_CXXMBD_HPP
#define CXXMBD_CXXMBD_HPP

#define CXXMBD_VER "1.0.1"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include "mbdutils.hpp"


namespace cxxmbd {
/*
 * template <std::size_t N>
 * using byte_block_t = unsigned char[N];
 *
 * template <std::size_t N>
 * struct binary_embed {
 *    std::string filename;
 *    byte_block_t<N> data;
 * };
 *
 * binary_embed<N> filename {
 *    "filename.ext", 0x01, 0x30, 0x60, ...
 * }
 */

    void
    read_binary_contents(std::vector<byte_t>& vec, const fs::path& filepath);

    void
    create_embeddable_contents(std::stringstream& ss, fs::path file);

    [[nodiscard]]
    std::stringstream
    generate_output_stream(const std::vector<fs::path>& paths);

    void
    dump_to_path(fs::path& path, std::stringstream& ss);

    [[nodiscard]]
    std::optional<std::size_t>
    embed_location(fs::path& path, std::string& str);

    void handle_cl_args(int input_count, char* input_values[]);

    // For use in dlls
    void handle_cl_args(argument_splitter& args);
}

#endif //CXXMBD_CXXMBD_HPP
