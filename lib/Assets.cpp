//
// Created by geoco on 19.02.2022.
//
#include <assets.h>

nlohmann::json Assets::ship_components(const std::string_view file_name) {
    std::ifstream file("assets/json/ship_components.json");
    return nlohmann::json::parse(file);
}