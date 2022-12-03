#ifndef CXXMBD_TEST_ANSI_COLOR_HPP
#define CXXMBD_TEST_ANSI_COLOR_HPP

#include <iostream>
#include <string_view>

namespace cxxmbd::ansi {
    struct ansi_base {
        std::string_view color { "\u001b[0m" };

        friend std::ostream& operator<<(std::ostream& os, const ansi_base& c) {
            return os << c.color;
        }
    };


#ifndef DISABLE_ANSI
    constexpr ansi_base reset {};
    constexpr ansi_base red { "\u001b[31;1m" };
    constexpr ansi_base green { "\u001b[32;1m" };
    constexpr ansi_base blue { "\u001b[34;1m" };
    constexpr ansi_base yellow { "\u001b[33;1m" };
    constexpr ansi_base cyan { "\u001b[36;1m" };
    constexpr ansi_base white { "\u001b[37;1m" };
#else
    constexpr ansi_base reset {};
    constexpr ansi_base red {};
    constexpr ansi_base green {};
    constexpr ansi_base blue {};
    constexpr ansi_base yellow {};
    constexpr ansi_base cyan {};
    constexpr ansi_base white {};
#endif
}

#endif //CXXMBD_TEST_ANSI_COLOR_HPP
