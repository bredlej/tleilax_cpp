//
// Created by geoco on 19.12.2021.
//

#include "Galaxy.h"
static constexpr auto seed_function = [](auto x, auto y, auto z) {
    return ((x + y) >> 1) * (x + y + 1) + y, ((x + z) >> 1) * (x + z + 1) + z;
};
void Galaxy::render() {
}
void Galaxy::recalculate() {
    pcg64 pcg;
    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                const auto chance = pcg(_star_chance);
                if (chance == 1) {
                    const auto star = registry.create();
                    registry.emplace<Coordinates>(star, x, y, z);
                }
            }
        }
    }
    auto view = registry.view<Coordinates>();
    view.each([](const auto &coordinates) {
        std::printf("Star at %d, %d, %d\n", coordinates.x, coordinates.y, coordinates.z);
    });
}
