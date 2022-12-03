#include "os.hpp"
#include <cxxmbd/mbdutils.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <type_traits>

#if defined(ON_WINDOWS)
#  include <windows.h>
#  include <strsafe.h>
#  include <locale>
#  define INVOKE_FUNCTION(fn, err)                                                      \
    [] <typename...TT>                                                                  \
    (TT&&...tt) {                                                                       \
        auto tmp = fn(std::forward<TT>(tt)...);                                         \
        if(tmp == err) {                                                                \
            if constexpr(not std::is_same_v<LPTSTR, char*>) {                           \
                using base_t = std::remove_pointer_t<LPTSTR>;                           \
                using lptstr_str = std::basic_string<base_t,                            \
                    std::char_traits<base_t>,                                           \
                    std::allocator<base_t>>;                                            \
                std::string input { #fn };                                              \
                lptstr_str output {};                                                   \
                auto& facet = std::use_facet<std::ctype<base_t>>(std::locale {});       \
                facet.widen(input.data(), input.data() + input.size(), output.data());  \
                throw_last_error(output.data());                                        \
            }                                                                           \
            else throw_last_error((LPTSTR) #fn);                                        \
        }                                                                               \
        return tmp;                                                                     \
    }
#elif defined(ON_UNIX)
#  include <unistd.h>
#  ifndef _GNU_SOURCE
     extern char** environ;
#  endif
#endif

namespace cxxmbd::os {
#if defined(ON_WINDOWS)

    static void
    throw_last_error(LPTSTR lpszFunction) {
        LPVOID lpMsgBuf;
        LPTSTR lpOutBuf;
        DWORD err_code = GetLastError();

        FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // NOLINT
                (LPTSTR) &lpMsgBuf, 0, NULL);  // NOLINT

        lpOutBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        std::size_t out_size = LocalSize(lpOutBuf) / sizeof(TCHAR);
        StringCchPrintf((LPTSTR)lpOutBuf,out_size,
                        TEXT("os error: %s failed with error %d; %s"),
                        lpszFunction, err_code, lpMsgBuf);

        if constexpr(std::is_same_v<LPTSTR, char*>) {
            std::string throw_buf { lpOutBuf };
            LocalFree(lpMsgBuf);
            LocalFree(lpOutBuf);
            throw custom_exception { throw_buf + "" };
        }
        else {
            using base_t = std::remove_pointer_t<LPTSTR>;
            auto& facet = std::use_facet<std::ctype<base_t>>(std::locale {});

            std::string throw_buf {};
            throw_buf.resize(out_size);
            std::transform(lpOutBuf, lpOutBuf + out_size, throw_buf.begin(),
                           [&](auto c) { return facet.narrow(c, '*'); });

            LocalFree(lpMsgBuf);
            LocalFree(lpOutBuf);
            throw custom_exception { throw_buf + "" };
        }
    }

    static void
    parse_envs(std::vector<std::string_view>& out, char* env_str) {
        for(;;) {
            std::string_view substr { env_str };
            if(substr == "") break;
            else {
                out.emplace_back(substr);
                env_str += substr.size() + 1;
            }
        }
    }

    [[nodiscard]]
    env_registry
    get_os_envs() {
        char* raw_envs = INVOKE_FUNCTION(GetEnvironmentStrings, nullptr)();
        std::vector<std::string_view> packed_envs {};
        parse_envs(packed_envs, raw_envs);

        env_registry env_map {};
        for(const auto& str : packed_envs) {
            std::size_t pos = str.find('=');
            if(pos == std::string_view::npos) env_map.add_invalid(str);
            else {
                std::string_view key = str.substr(0, pos);
                std::string_view value = str.substr(pos + 1);
                if(key == "" || value == "") env_map.add_invalid(str);
                else env_map.add_env(key, value);
            }
        }

        return env_map;
    }


    void
    enable_ansi_color() {
        HANDLE console_input = INVOKE_FUNCTION(GetStdHandle, INVALID_HANDLE_VALUE)(STD_OUTPUT_HANDLE);
        INVOKE_FUNCTION(SetConsoleMode, 0)(console_input, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

#elif defined(ON_UNIX)

    static std::size_t
    get_environ_count() {
        char** envs = environ;
        std::size_t count = 0;
        while(*envs) {
            ++envs;
            ++count;
        }
        return count;
    }

    [[nodiscard]]
    env_registry
    get_os_envs() {
        auto env_count = get_environ_count();
        std::vector<std::string> packed_envs { environ, environ + env_count };

        env_registry env_map {};
        for(const auto& str : packed_envs) {
            std::size_t pos = str.find('=');
            if(pos == std::string::npos) env_map.add_invalid(str);
            else {
                std::string key = str.substr(0, pos);
                std::string value = str.substr(pos + 1);
                env_map.add_env(key, value);
            }
        }

        return env_map;
    }


    void
    enable_ansi_color() {}

#else

    [[nodiscard]]
    env_registry
    get_os_envs() {
        using namespace std::string_literals;
        throw custom_exception {
            "os error: cannot determine the environment variables for system \""s + ON_UNKNOWN + "\"."
        };
    }


    void
    enable_ansi_color() {}

#endif

    [[nodiscard]]
    std::string
    get_env(std::string env_var) {
        auto* result = std::getenv(env_var.c_str());
        if(not result) return "<null>";
        else return { result };
    }
}
