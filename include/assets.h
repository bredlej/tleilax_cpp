//
// Created by geoco on 19.02.2022.
//

#ifndef TLEILAX_ASSETS_H
#define TLEILAX_ASSETS_H
#include <fstream>
#include <nlohmann/json.hpp>

struct files {
    static constexpr std::string_view ship_components = "assets/json/ship_components.json";
};

struct Assets {
    nlohmann::json ship_components(const std::string_view file_name);
};

#endif//TLEILAX_ASSETS_H
