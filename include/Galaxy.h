//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <components.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <raylib.h>

class Galaxy {
public:
    Galaxy() : _camera(_initialize_camera({0., 0., 0.}, 110., 10., 90., 90.)){};
    ~Galaxy() = default;

    void render() const;
    void update();
    void populate();

private:
    entt::registry _registry;
    Vector3 _offset{0., 0., 0.};
    const Vector3 _visible_size{75, 50, 75};
    static constexpr uint32_t _star_occurence_chance = 5000;
    Camera _camera;

    Camera _initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                              const float horizontalDistance, const float horizontalAngle,
                              const float verticalAngle);

    void _render_visible() const;
};


#endif//TLEILAX_GALAXY_H
