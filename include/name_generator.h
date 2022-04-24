//
// Created by geoco on 24.04.2022.
//

#ifndef TLEILAX_NAME_GENERATOR_H
#define TLEILAX_NAME_GENERATOR_H
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>
#include <pcg/pcg_random.hpp>
#include <cstdint>

enum class NameType : uint8_t {
    Greek = 0, Indian
};

const std::vector<std::string> star_suffixes {
        " I ",
        " II ",
        " III ",
        " IV ",
        " X ",
        " Polaris ",
        " Prime "
};
inline std::vector<std::string> load_from_file(const std::string &file_name) {
    std::vector<std::string> names;
    std::ifstream file_handler;
    file_handler.open(file_name);
    if (file_handler.is_open()) {
        std::string line;
        while (std::getline(file_handler, line)) {
            names.emplace_back(line);
        }
        file_handler.close();
    }
    return names;
}
class NameGenerator {
public:
    NameGenerator() {
        _names[NameType::Greek] = load_from_file("assets/names/greek");
        _names[NameType::Indian] = load_from_file("assets/names/indian");
    }
    template<typename T>
    std::string get_random_name(pcg32 &pcg);
private:
    std::unordered_map<NameType, std::vector<std::string>> _names;
};
#endif//TLEILAX_NAME_GENERATOR_H
