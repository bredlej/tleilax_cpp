//
// Created by geoco on 19.02.2022.
//

#ifndef TLEILAX_ASSETS_H
#define TLEILAX_ASSETS_H
#include <fstream>
#include <nlohmann/json.hpp>

struct files {
    static constexpr const char * ship_components = "assets/json/ship_components.json";
};

struct Assets {
public:
    explicit Assets(const std::string &ship_components_path) : ship_components {Assets::load_from_file(ship_components_path)} {};
    static nlohmann::json load_from_file(const std::string& file_name);

    nlohmann::json get_ship_components() const;
private:
    nlohmann::json ship_components;
};

#endif//TLEILAX_ASSETS_H
