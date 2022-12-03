#include "cxxmbd.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

namespace cxxmbd {
    void
    read_binary_contents(std::vector<byte_t>& vec, const fs::path& filepath) {
        using namespace std::string_literals;
        if(fs::exists(filepath)) {
            std::ifstream is(filepath, std::ios::binary);
            if(is.is_open()) {
                is.unsetf(std::ios::skipws);

                std::streampos size;

                is.seekg(0, std::ios::end);
                size = is.tellg();
                is.seekg(0, std::ios::beg);

                std::istream_iterator<byte_t> start(is), end;
                vec.reserve(size);
                vec.insert(vec.cbegin(), start, end);
                is.close();
            }
            else throw custom_exception{ "file \""s + filepath.filename().string() + "\" could not be opened." };
        }
        else throw custom_exception{ "file \""s + filepath.filename().string() + "\" not found." };
    }


    static void
    output_byte_to_stream(std::stringstream& ss, byte_t b) {
        ss << "0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned>(b);
    }

    void
    create_embeddable_contents(std::stringstream& ss, fs::path file) {
        std::vector<byte_t> bin;
        try { read_binary_contents(bin, file); }
        catch(std::exception& e) { std::cout << e.what() << "\n"; return; }

        ss << "\nconstexpr binary_embed<" << bin.size() << "> " << file.stem().string() << " {\n";
        ss << "\t" << file.filename() << ", { ";

        std::span<byte_t> to_loop { bin.data(), bin.size() - 1 };
        for(auto c : to_loop) {
            output_byte_to_stream(ss, c);
            ss << ", ";
        }
        output_byte_to_stream(ss, bin.back());
        ss << " }\n};\n";
    }

    [[nodiscard]]
    std::stringstream
    generate_output_stream(const std::vector<fs::path>& paths) {
        std::stringstream output_stream;
        output_stream << "EMBED_START\n";
        for(const auto& path : paths) create_embeddable_contents(output_stream, path);
        output_stream << "\nEMBED_END\n";
        return output_stream;
    }


    void
    dump_to_path(fs::path& path, std::stringstream& ss) {
        try {
            if(path.empty()) throw custom_exception{ "error: cannot output to empty path." };
            path.replace_extension(".txt");

            if(!path.has_parent_path()) {
                fs::path curr = fs::current_path();
                path = (curr /= path);
            }

            if(!fs::exists(path.parent_path())) {
                fs::create_directories(path.parent_path());
            }

            std::ofstream of(path);
            if(!of.is_open()) throw custom_exception { std::string{"error: could not open \""} + path.string() + "\".\n" };

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
    strspan
    embed_location(fs::path& path, std::string& str) {
        using namespace std::string_literals;
        if(not fs::exists(path)) throw custom_exception{ "error: output file could not be found." };

        std::ifstream is { path };
        std::size_t embed_point;
        std::size_t start_location;
        std::size_t end_location;

        if(is.is_open()) {
            std::stringstream ss;
            ss << is.rdbuf();
            str = ss.str();

            embed_point = str.find("EMBED_POINT");
            start_location = str.find("EMBED_START");
            end_location = str.find("EMBED_END");

            if(embed_point != std::string::npos) {
                is.close();
                if(start_location != std::string::npos || end_location != std::string::npos) {
                    throw custom_exception {
                        "error: 'EMBED_POINT' found with embed spans in \""s + path.filename().string() + "\""
                    };
                }
                return { .pos = start_location, .len = 11 };
            }

            if(start_location != std::string::npos && end_location != std::string::npos) {
                is.close();
                auto length = (end_location + 9) - start_location;
                return { .pos = start_location, .len = length };
            }

            is.close();
            throw custom_exception{ "error: could not locate embed point in \""s + path.filename().string() + "\"." };
        }
        throw custom_exception{ "error: could not open file \""s + path.filename().string() + "\"." };
    }


    void
    handle_cl_args(int input_count, char* input_values[]) {
        if(input_count < 2) {
            throw custom_exception{ "Unknown command. Run 'cxxmbd --help' for more information." };
        }

        std::string command { input_values[1] };
        std::transform(command.cbegin(), command.cend(),
                       command.begin(), [](char c) { return std::tolower(c); });

        if(command == "--help" || command == "-help" || command == "-h" || command == "-usage") {
            std::cout << "Options: \n";
            std::cout << "\t--help, -help, -h, -usage" << "                        = Print information and exit.\n";
            std::cout << "\t--check, -c <path-to-output>" << "                     = Check if file is embeddable and exit.\n";
            std::cout << "\t--dump, -d <name> <path-to-source(s)>" << "            = Output data to file and exit.\n";
            std::cout << "\t--embed, -e <path-to-output> <path-to-source(s)>"
                      << " = Locate existing file, output data at embed point and exit.\n";
            std::cout << "\t--version, -v" << "                                    = Print program version and exit.\n\n";
        }
        else if(command == "--check" || command == "-c") {
            if(input_count < 3) {
                throw custom_exception{ "Error: no source file specified.\nRun 'cxxmbd --help' for more information." };
            }
            fs::path path { input_values[2] };
            std::string filebuf;
            try {
                auto opt = embed_location(path, filebuf);
                std::cout << "The file \"" << path.string() << "\" is embeddable.\n\n";
            }
            catch(std::exception& e) {
                std::cout << "The file \"" << path.string() << "\" is not embeddable.\n\n";
            }
        }
        else if(command == "--dump" || command == "-d") {
            if(input_count < 3) {
                throw custom_exception{ "Error: no output file specified.\nRun 'cxxmbd --help' for more information." };
            }
            else if(input_count < 4) {
                throw custom_exception{ "Error: no source file(s) specified.\nRun 'cxxmbd --help' for more information." };
            }

            fs::path opath { input_values[2] };
            std::vector<fs::path> ipaths;
            for(int idx = 3; idx < input_count; ++idx) ipaths.emplace_back(input_values[idx]);

            auto ss = generate_output_stream(ipaths);
            dump_to_path(opath, ss);
        }
        else if(command == "--embed" || command == "-e") {
            if(input_count < 3) {
                throw custom_exception{ "Error: no output file specified.\nRun 'cxxmbd --help' for more information." };
            }
            else if(input_count < 4) {
                throw custom_exception{ "Error: no source file(s) specified.\nRun 'cxxmbd --help' for more information." };
            }

            fs::path opath { input_values[2] };
            std::vector<fs::path> ipaths;
            std::string filebuf;
            for(int idx = 3; idx < input_count; ++idx) ipaths.emplace_back(input_values[idx]);

            auto location = embed_location(opath, filebuf);
            auto ss { generate_output_stream(ipaths) }; //+11
            auto new_file { filebuf.replace(location.pos, location.len, ss.str()) };

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

    void
    handle_cl_args(argument_splitter& args) {
        handle_cl_args(args.argc, static_cast<char**>(args.argv));
    }
}
