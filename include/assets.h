//
// Created by geoco on 19.02.2022.
//

#ifndef TLEILAX_ASSETS_H
#define TLEILAX_ASSETS_H
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace assets {
    enum class types {
        weapon,
        shield,
        engine,
        cargo,
        hull
    };
    static std::unordered_map<types, const char *> names = {
            {types::weapon, "weapons"},
            {types::engine, "engines"},
            {types::shield, "shields"},
            {types::hull, "hulls"},
            {types::cargo, "cargo"}};
} // namespace assets

struct files {
    static constexpr const char *ship_components = "assets/json/ship_components.json";
    static constexpr const char *ships = "assets/json/ships.json";
};

class Assets {
public:
    explicit Assets(const std::string &ship_components_path, const std::string &ships_path) noexcept
        : ship_components{Assets::load_from_file(ship_components_path)},
          ships{Assets::load_from_file(ships_path)} {};
    Assets() noexcept = delete;
    Assets(const Assets&) noexcept = delete;
    Assets(Assets&&) noexcept = delete;
    Assets& operator=(const Assets&) noexcept = delete;
    Assets& operator=(Assets&&) noexcept = delete;
    ~Assets() noexcept = default;

    static nlohmann::json load_from_file(const std::string &file_name);

    [[nodiscard]] nlohmann::json get_ship_components() const;
    [[nodiscard]] nlohmann::json get_ships() const;

private:
    nlohmann::json ship_components;
    nlohmann::json ships;
};

#endif//TLEILAX_ASSETS_H
