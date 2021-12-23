//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <pcg/pcg_random.hpp>
#include <components.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <raylib.h>

class Galaxy {
public:
    Galaxy() : _camera(_initialize_camera({0., 0., 0.}, 100., 10., 90., 45.)) {};
    ~Galaxy() = default;

    void render();
    void populate();
private:
    entt::registry _registry;
    Vector3 _offset{0.,0.,0.};
    const Vector3 _visible_size{50, 50, 50};
    static constexpr int64_t _star_occurence_chance = 1000;
    Camera _camera;

    Camera _initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                            const float horizontalDistance, const float horizontalAngle,
                            const float verticalAngle) {
        Camera camera = {0};
        camera.target = cameraInitialPosition;
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        camera.position.x = horizontalDistance * cosf(horizontalAngle * PI / 180.0f);
        camera.position.z = horizontalDistance * sinf(horizontalAngle * PI / 180.0f);
        camera.position.y = cameraDistance * sinf(verticalAngle * PI / 180.0f);
        return camera;
    }

    void _render_visible(const float distance);
};


#endif//TLEILAX_GALAXY_H
