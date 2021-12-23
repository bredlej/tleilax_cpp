//
// Created by geoco on 19.12.2021.
//

#include "Galaxy.h"

constexpr auto seed_function = [](uint32_t x, uint32_t y, uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

constexpr auto local_to_global_coords = [](auto coordinates, auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};

constexpr auto is_star_at = [](auto x, auto y, auto z, auto chance) {
    return 0 == chance;
};

void Galaxy::populate() {
    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                pcg32 pcg(seed_function(x, y, z));
                if (is_star_at(x, y, z, pcg(_star_occurence_chance))) {
                    const auto star = _registry.create();
                    _registry.emplace<Vector3>(star, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    _registry.emplace<StarColor>(star, StarColor{static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), 255});
                }
            }
        }
    }
}

void Galaxy::_render_visible() const {

    BeginMode3D(_camera);
    DrawCubeWires({0., 0., 0.}, _visible_size.x, _visible_size.y, _visible_size.z, YELLOW);

    _registry.view<Vector3, StarColor>().each([&](const Vector3 &coords, const StarColor color) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        DrawSphere(star_coords, 1., {color.r, color.g, color.b, color.a});
    });
    EndMode3D();
}

Camera Galaxy::_initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                                  const float horizontalDistance, const float horizontalAngle,
                                  const float verticalAngle) {
    Camera camera;
    camera.target = cameraInitialPosition;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position.x = horizontalDistance * cosf(horizontalAngle * PI / 180.0f);
    camera.position.z = horizontalDistance * sinf(horizontalAngle * PI / 180.0f);
    camera.position.y = cameraDistance * sinf(verticalAngle * PI / 180.0f);
    return camera;
}

void Galaxy::render() const {
    BeginDrawing();
    ClearBackground(BLACK);
    _render_visible();
    EndDrawing();
}
