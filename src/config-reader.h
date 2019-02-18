//
// Created by yihan on 2/15/19.
//

#ifndef SPACEHUB_CONFIG_READER_H
#define SPACEHUB_CONFIG_READER_H

#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "dev_tools.h"

namespace SpaceH {

    class ConfigReader {
    public:
        explicit ConfigReader(std::string const &file_name, char divider = '=', char commenter = '#') {
            std::fstream file(file_name);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

                    if (line[0] == commenter || line.empty())
                        continue;

                    auto split = line.find(divider);
                    auto key = line.substr(0, split);
                    auto val = line.substr(split + 1);
                    map_[key] = val;
                }
            } else {
                SPACEHUB_ABORT("Cannot open the configure file: ", file_name, "\r\n");
            }
        }

        explicit ConfigReader(char const *file_name, char divider = '=', char commenter = '#')
                : ConfigReader(std::string(file_name), divider, commenter) {};

        template<typename T>
        T get(std::string const &key) {
            auto iskey = (map_.end() != map_.find(key));
            if (iskey) {
                std::stringstream ss(map_[key]);
                T value;
                ss >> value;
                return value;
            } else {
                SPACEHUB_ABORT("Invalid key for configure file!");
            }
        }

        friend std::ostream &operator<<(std::ostream &os, ConfigReader const &config) {
            for (auto&[key, value] : config.map_) {
                SpaceH::print(os, key, '=', value, '\n');
            }
        }

    private:
        std::unordered_map<std::string, std::string> map_;
    };

    template<typename ...Args>
    void read_command_line(int argc, char **argv, Args &&...args) {
        constexpr size_t n = sizeof...(Args);

        if (argc != n + 1) {
            SPACEHUB_ABORT("Wrong args number!");
        } else {
            std::stringstream ss;
            for (int i = 1; i < argc; ++i) {
                ss << argv[i] << ' ';
            }
            ((ss >> std::forward<Args>(args)),...);
        }
    }
}


#endif //SPACEHUB_CONFIG_READER_H