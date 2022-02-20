//
// Created by geoco on 19.02.2022.
//
#include <assets.h>

nlohmann::json Assets::load_from_file(const std::string &file_name) {
    std::ifstream file(file_name.c_str());
    return nlohmann::json::parse(file);
}

nlohmann::json Assets::get_ship_components() const {
    return ship_components;
}

nlohmann::json Assets::get_ships() const {
    return ships;
}
