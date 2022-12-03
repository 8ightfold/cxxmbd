#include "core.hpp"
#include <cstdint>
#include <stack>

#include <cxxmbd/cxxmbd.hpp>

#ifdef CXXMBD_ENABLE_ENV_DEBUG
#  define ENV_DEBUG(data) do { data } while(0);
#else
#  define ENV_DEBUG(data)
#endif

namespace cxxmbd {
    env::env(toml::parse_result& config) : config_settings(get_settings(config)) {
        using namespace std::string_literals;
        if(config_settings.include_os_env) config_env = os::get_os_envs();
        if(config.contains("env")) {
            auto env_table = get_this_as(config, "env", toml::table);
            for(const auto& [k, v] : env_table) {
                if(auto* value = v.as_string()) config_env.add_env(k.str(), (*value)->c_str());
                else throw custom_exception {
                            "error: incorrect type used for environment variable \""s + std::string{ k.str() } + "\"."
                    };
            }
        }
    }

    settings
    get_settings(toml::parse_result& config) {
        if(config.contains("settings")) {
            settings values {};
            auto settings_table = get_this_as(config, "settings", toml::table);

            if(settings_table.contains("require_version")) {
                values.require_version = true;
                values.version = program_version { *get_this_as(settings_table, "require_version", std::string) };
            }
            if(settings_table.contains("include_os_env")) {
                values.include_os_env = *get_this_as(settings_table, "include_os_env", bool);
            }
            if(settings_table.contains("print_parsed")) {
                values.print_parsed = *get_this_as(settings_table, "print_parsed", bool);
            }
            if(settings_table.contains("print_env")) {
                values.print_env = *get_this_as(settings_table, "print_env", bool);
            }
            if(settings_table.contains("print_successful")) {
                values.print_successful = *get_this_as(settings_table, "print_successful", bool);
            }

            return values;
        }
        else return {};
    }

    void
    verify_settings(const env& e) {
        auto& config_settings = e.config_settings;
        if(config_settings.require_version) {
            program_version this_ver { CXXMBD_VER };
            if(not (this_ver > config_settings.version)) {
                throw custom_exception {
                        "error: program version "s + CXXMBD_VER
                        + " is lower than the required version "s + config_settings.version.str() + "."
                };
            }
        }
    }


    static std::size_t
    replace_env(const env& vars, std::string& str, const strspan range) {
        std::int64_t front = range.pos + 2;
        std::int64_t back = range.pos + range.len - 2;

        while(str[front] == ' ') ++front;
        while(str[back] == ' ') --back;

        if(front > back) throw custom_exception {
            "error: parsing of \""s + str + "\" failed; front > back."
        };

        std::string env_name = str.substr(front, back - front + 1);
        std::string replacement = vars[env_name];

        ENV_DEBUG(
            std::cout << "  name: " << env_name << '\n';
            std::cout << "  replace: " << replacement << '\n';
            std::cout << "  range: { .pos = " << range.pos << ", .len = " << range.len << " }\n";
        );

        str.replace(range.pos, range.len, replacement);
        ENV_DEBUG( std::cout << "  new: " << str << '\n'; );
        return range.pos + (replacement.size() - range.len);
    }

    static void
    add_env_vars(const env& vars, std::string& str) {
        std::stack<std::int64_t> open_brackets {};
        std::string old_str { str };
        std::int64_t current_location = 0;

        auto curr = [&]() -> char { return str[current_location]; };
        auto next = [&]() -> char {
            return (current_location + 1 >= str.size()) ? '\0' : str[current_location + 1];
        };

        for(; current_location < str.size(); ++current_location) {
            if(curr() == '$' && next() == '{') {
                open_brackets.push(current_location);
            }
            else if(curr() == '}') {
                if(not open_brackets.empty()) {
                    std::size_t pos = open_brackets.top();
                    open_brackets.pop();
                    ENV_DEBUG( std::cout << str.substr(pos, current_location - pos + 1) << ":\n"; );
                    current_location = replace_env(vars, str, { .pos = pos, .len = current_location - pos + 1 });
                }
                else throw custom_exception {
                    "error: '}' found without '${' in \""s + old_str + "\"."
                };
            }
        }

        if(not open_brackets.empty()) throw custom_exception {
            "error: '${' found without '}' in \""s + old_str + "\"."
        };
    }


    void
    parse_config(const fs::path& path) {
        toml::parse_result config = toml::parse_file(path.c_str());
        if(config.contains("embed")) {
            env config_env { config };

            if(config_env.config_settings.print_env) {
                for(const auto& [k, v] : config_env.config_env.get_envs()) {
                    std::cout << ansi::blue << k;
                    std::cout << ansi::reset << " = ";
                    std::cout << v << '\n';
                }
                std::cout << '\n';
            }

            verify_settings(config_env);
            auto embed_table { get_this_as(config, "embed", toml::table) };

            for(const auto& [k, v] : embed_table) {
                fs::path embed_path { path.parent_path() /= k.str() };

                auto sources = check_node(v.as_array(), std::string{ k.str() } + " source values");
                std::vector<fs::path> parsed_sources {};
                parsed_sources.reserve(sources.size());

                for(const auto& source : sources) {
                    std::string value {
                            check_node(source.as_string(),
                            std::string{ k.str() } + " source")->c_str()
                    };
                    add_env_vars(config_env, value);
                    parsed_sources.push_back(value);
                }

                if(config_env.config_settings.print_parsed) {
                    std::cout << k.str() << ": { ";
                    for(const auto& parsed_source: parsed_sources) {
                        std::cout << parsed_source << ' ';
                    }
                    std::cout << "}\n";
                }


                std::string filebuf {};
                auto range { embed_location(embed_path, filebuf) };
                auto ss { generate_output_stream(parsed_sources) };
                filebuf.replace(range.pos, range.len, ss.str());

                std::ofstream output(embed_path);
                if(output.is_open()) {
                    output << filebuf;
                    output.close();
                    if(config_env.config_settings.print_successful) {
                        std::cout << "output to " << embed_path << " successfully.\n";
                    }
                }
            }
        }
    }


    void
    forward_cl_args(int argc, char* argv[]) {
        using namespace std::string_literals;

        try { os::enable_ansi_color(); }
        catch(std::exception& e) { std::cout << e.what() << '\n'; }

        fs::path path {};
        if(argc > 1) {
            path = argv[1];
            if(fs::is_directory(path)) {
                path /= "mbdconfig.toml";
            }
        }
        else path = fs::current_path() / "mbdconfig.toml";

        if(not fs::exists(path)) throw custom_exception{
            "error: config file \""s + path.string() + "\" could not be found."
        };
        parse_config(path);
    }

    void
    forward_cl_args(argument_splitter& args) {
        forward_cl_args(args.argc, static_cast<char**>(args.argv));
    }
}