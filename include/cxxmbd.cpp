#include "cxxmbd.hpp"

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

namespace cxxmbd {
    void
    read_binary_contents(std::vector<byte_t>& vec, const fs::path& filepath)
    {
        if(fs::exists(filepath)) {
            std::ifstream is(filepath, std::ios::binary);
            is.unsetf(std::ios::skipws);

            std::streampos size;

            is.seekg(0, std::ios::end);
            size = is.tellg();
            is.seekg(0, std::ios::beg);

            std::istream_iterator<byte_t> start(is), end;
            vec.reserve(size);
            vec.insert(vec.cbegin(), start, end);
        }
        else throw custom_exception{ std::string{"File \""} + filepath.filename().string() + "\" not found." };
    }

    void
    create_embeddable_contents(std::stringstream& ss, fs::path file)
    {
        std::vector<byte_t> bin;
        try { read_binary_contents(bin, file); }
        catch(std::exception& e) { std::cout << e.what() << "\n"; return; }

        ss << "binary_embed<" << (bin.size() + 1) << "> " << file.stem().string() << " {\n";
        ss << "\t" << file.filename() << ", { ";

        std::size_t n_written = 0;
        for(auto c : bin) {
            ss << "0x" << std::setfill('0') << std::setw(2) << std::hex
               << static_cast<unsigned>(c) << ", ";
        }
        ss << "0x00 }\n};\n\n";
    }

    [[nodiscard]]
    std::stringstream
    generate_output_stream(const std::vector<fs::path>& paths)
    {
        std::stringstream output_stream;
        for(const auto& path : paths) create_embeddable_contents(output_stream, path);
        return output_stream;
    }

    void
    dump_to_path(fs::path& path, std::stringstream& ss) {
        try {
            if(path.empty()) throw custom_exception{ "Cannot output to empty path." };
            path.replace_extension(".txt");

            if(!path.has_parent_path()) {
                fs::path curr = fs::current_path();
                path = (curr /= path);
            }

            if(!fs::exists(path.parent_path())) {
                fs::create_directories(path.parent_path());
            }

            std::ofstream of(path);
            if(!of.is_open()) throw custom_exception { std::string{"Could not open \""} + path.string() + "\".\n" };

            of << ss.rdbuf();
            of << std::flush;
            of.close();

            std::cout << "Output successful. Dumped to \"" << path.string() << "\".\n\n";
        }
        catch(std::exception& e) {
            std::cout << e.what();
        }
    }

    [[nodiscard]]
    std::optional<std::size_t>
    embed_location(fs::path& path, std::string& str) {
        if(!fs::exists(path)) return {};

        std::ifstream is { path };
        std::size_t location;

        if(is.is_open()) {
            std::stringstream ss;
            ss << is.rdbuf();
            str = ss.str();
            if(location = str.find("EMBED_POINT"); location != std::string::npos) {
                is.close();
                return { location };
            }
            is.close();
        }
        return {};
    }

    void handle_cl_args(int input_count, char* input_values[])
    {
        if(input_count < 2) {
            std::cout << "Unknown command. Run 'cxxmbd --help' for more information.\n\n";
            return;
        }

        std::string command { input_values[1] };
        std::transform(command.cbegin(), command.cend(),
                       command.begin(), [](char c) { return std::tolower(c); });

        if(command == "--help" || command == "-help" || command == "-h" || command == "-usage") {
            std::cout << "Options: \n";
            std::cout << "\t--help, -help, -h, -usage" << "                        = Print information and exit.\n";
            std::cout << "\t--check, -c <path-to-output>" << "                     = Check if file is embeddable and exit.\n";
            std::cout << "\t--dump, -d <name> <path-to-source>..." << "            = Output data to file and exit.\n";
            std::cout << "\t--embed, -e <path-to-output> <path-to-source(s)>"
                      << " = Locate existing file, locate 'EMBED_POINT', and output data if found.\n";
            std::cout << "\t--version, -v" << "                                    = Print program version and exit.\n\n";
        }
        else if(command == "--check" || command == "-c") {
            if(input_count < 3) {
                std::cout << "Error: no source file specified.\n";
                std::cout << "Run 'cxxmbd --help' for more information.\n\n";
                return;
            }
            fs::path path { input_values[2] };
            std::string filebuf;
            auto opt = embed_location(path, filebuf);

            std::cout << "The file \"" << path.string() << "\" "
                      << (opt.has_value() ? "is " : "is not ") << "embeddable.\n\n";
        }
        else if(command == "--dump" || command == "-d") {
            if(input_count < 3) {
                std::cout << "Error: no output file specified.\n";
                std::cout << "Run 'cxxmbd --help' for more information.\n\n";
                return;
            }
            else if(input_count < 4) {
                std::cout << "Error: no source files specified.\n";
                std::cout << "Run 'cxxmbd --help' for more information.\n\n";
                return;
            }

            fs::path opath { input_values[2] };
            std::vector<fs::path> ipaths;
            for(int idx = 3; idx < input_count; ++idx) ipaths.emplace_back(input_values[idx]);

            auto ss = generate_output_stream(ipaths);
            dump_to_path(opath, ss);
        }
        else if(command == "--embed" || command == "-e") {
            if(input_count < 3) {
                std::cout << "Error: no output file specified.\n";
                std::cout << "Run 'cxxmbd --help' for more information.\n\n";
                return;
            }
            else if(input_count < 4) {
                std::cout << "Error: no source files specified.\n";
                std::cout << "Run 'cxxmbd --help' for more information.\n\n";
                return;
            }

            fs::path opath { input_values[2] };
            std::vector<fs::path> ipaths;
            std::string filebuf;
            for(int idx = 3; idx < input_count; ++idx) ipaths.emplace_back(input_values[idx]);

            auto opt = embed_location(opath, filebuf);
            if(!opt.has_value()) {
                std::cout << "Could not locate 'EMBED_POINT'.\n\n";
                return;
            }
            auto ss = generate_output_stream(ipaths); //+11
            auto new_file = filebuf.replace(opt.value(), 11, ss.str());

            std::ofstream output(opath);
            if(output.is_open()) {
                output << new_file;
                output.close();
                std::cout << "Embedded in \"" << opath.string() << "\" successfully.\n\n";
            }
        }
        else if(command == "--version" || command == "-v") {
            std::cout << "cxxmbd version " << CXXMBD_VER << "\n\n";
        }
        else {
            std::cout << "Unknown command. Run 'cxxmbd --help' for more information.\n\n";
        }
    }
}
