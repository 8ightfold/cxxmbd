#ifndef CXXMBD_TEST_OS_HPP
#define CXXMBD_TEST_OS_HPP

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cxxmbd::os {
    struct env_registry {
        void
        add_invalid(const std::string& str) {
            invalid_envs.emplace_back(str);
        }

        void
        add_invalid(std::string_view str) {
            invalid_envs.emplace_back(str);
        }

        void
        add_env(const std::string& key, const std::string& value) {
            envs.emplace(key, value);
        }

        void
        add_env(std::string_view key, std::string_view value) {
            envs.emplace(key, value);
        }


        auto& get_envs()  {
            return envs;
        }

        auto& get_invalid()  {
            return invalid_envs;
        }

        auto& operator[](std::string str) const {
            if(envs.contains(str)) return envs.at(str);
            else return null_value;
        }

    private:
        std::vector<std::string> invalid_envs;
        std::map<std::string, std::string> envs;
        std::string null_value { "" };
    };

    [[nodiscard]]
    env_registry
    get_os_envs();

    [[nodiscard]]
    std::string
    get_env(std::string);

    void
    enable_ansi_color();
}

#endif //CXXMBD_TEST_OS_HPP
