#ifndef CXXMBD_TEST_CORE_HPP
#define CXXMBD_TEST_CORE_HPP

#include <toml.hpp>
#include <cttypeid.hpp>
#include <cxxmbd/mbdutils.hpp>
#include <cxxmbd/os.hpp>
#include <cxxmbd/ansi_color.hpp>

namespace cxxmbd {
    using namespace std::string_literals;

    struct program_version {
        program_version() = default;

        program_version(std::string version) : data(version) {
            std::size_t first = version.find('.');
            std::size_t second = version.rfind('.');
            if((first == std::string::npos || second == std::string::npos) || first == second) {
                throw custom_exception { "error: invalid version number "s + version + "." };
            }
            else {
                major = std::stoull(version.substr(0, first));
                minor = std::stoull(version.substr(first + 1, second - first));
                patch = std::stoull(version.substr(second + 1));
            }
        }

        const std::string&
        str() const { return data; }

        friend bool
        operator>(const program_version& lhs, const program_version& rhs) {
            if(lhs.major >= rhs.major && lhs.minor >= rhs.minor && lhs.patch >= rhs.patch) return true;
            else if (lhs.major >= rhs.major && lhs.minor > rhs.minor) return true;
            else if (lhs.major > rhs.major) return true;
            else return false;
        }

    private:
        std::string data;
        std::size_t major;
        std::size_t minor;
        std::size_t patch;
    };

    struct settings {
        bool require_version = false;
        program_version version;
        bool include_os_env = false;
        bool print_env = false;
        bool print_parsed = false;
        bool print_successful = false;
    };

    struct env {
        settings config_settings;
        os::env_registry config_env;

        env(toml::parse_result&);

        auto&
        operator[](std::string str) const {
            using namespace std::string_literals;
            auto& env { config_env[str] };
            if(env != "") return env;
            else throw custom_exception { "error: environment variable \""s + str + "\" not found." };
        }
    };

    template <typename T>
    auto
    get_as(auto node, std::string name) {
        auto* ret = node.template as<T>();
        if(ret) return *ret;
        else {
            std::string type_name { cttypeid<T>.name() };
            throw custom_exception { "error: incorrect type \""s + type_name + "\" used for \"" + name + "\"." };
        }
    }

    auto
    check_node(auto* node, std::string name) {
        if(node) return *node;
        else throw custom_exception {
                    "error: incorrect type used for \""s + name + "\"."
            };
    }

#define get_this_as(config, value, type) cxxmbd::get_as<type>(config[value], value)

    void
    verify_settings(const env& e);

    void
    forward_cl_args(argument_splitter& args);

    void
    forward_cl_args(int argc, char* argv[]);

    settings
    get_settings(toml::parse_result& config);
}

#endif //CXXMBD_TEST_CORE_HPP
