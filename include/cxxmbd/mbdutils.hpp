#ifndef CXXMBD_MBDUTILS_HPP
#define CXXMBD_MBDUTILS_HPP

#include <exception>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#  define $on_windows
#endif

#ifdef $on_windows
#include <windows.h>
#include <codecvt>
#include <locale>
#endif

#define EMBED_POINT

template <std::size_t N>
using byte_block_t = unsigned char[N];

template <std::size_t N>
struct binary_embed {
    std::string_view filename;
    byte_block_t<N> data;
};

namespace cxxmbd {
    namespace fs = std::filesystem;
    using byte_t = unsigned char;

    struct custom_exception : std::exception {
        explicit custom_exception(const std::string&& str) : data(str) {}

        [[nodiscard]]
        const char*
        what() const noexcept override {
            return data.c_str();
        }
    private:
        const std::string data;
    };

    // For windows support
    struct argument_splitter
    {
        struct argv_t
        {
            using data_t = char**;

            void create(std::vector<std::string>& argv_in) {
                data = new char*[argv_in.size()];
                unsigned idx = 0;
                for(auto& str : argv_in) {
                    data[idx] = str.data();
                    ++idx;
                }
            }

            explicit operator data_t() const {
                return data;
            }

            ~argv_t() { delete[] data; }

            data_t data = nullptr;
        };

#ifdef $on_windows
        argument_splitter(const std::wstring cl) {
            LPWSTR* raw_wargv = CommandLineToArgvW(cl.c_str(), &argc);
            std::vector<std::wstring> wargv { raw_wargv, raw_wargv + argc };
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

            for(auto& wstr : wargv) {
                argv_underlying.push_back(converter.to_bytes(wstr));
            }

            argv.create(argv_underlying);
        }
#endif

        argument_splitter() = default;
        argument_splitter(const argument_splitter&) = delete;
        argument_splitter(argument_splitter&&) = delete;

        int argc = 0;
        argv_t argv;

    private:
        std::vector<std::string> argv_underlying { fs::current_path().string() };
    };
}

#endif //CXXMBD_MBDUTILS_HPP
