//
// Created by geoco on 26.07.2022.
//
#include <battle.hpp>
#include <raylib_extension.h>
void battle::Battle::render() {
    BeginDrawing();
    ClearBackground(Colors::col_0);
    _render_visible();
    DrawText("Battle", 50, 50, 50, Colors::col_10);
    char attacker[10], opponent[10];
    std::sprintf(attacker, "%u", _attacker);
    std::sprintf(opponent, "%u", _opponent);
    DrawText(attacker, 50, 100, 50, Colors::col_12);
    DrawText(opponent, 50, 150, 50, Colors::col_12);

    DrawFPS(1200, 10);
    _draw_ui();
    EndDrawing();
}

void battle::Battle::update() {
    if (IsKeyDown(KEY_BACKSPACE)) {
        _core->dispatcher.enqueue<PlayerBattleEndEvent>();
    }
    if (IsKeyPressed(KEY_D)) {
        _open_demo = !_open_demo;
    }
    _core->dispatcher.update();
    //UpdateCamera(&_camera);
}

void battle::Battle::_draw_ui() {
    rlImGuiBegin();
    bool open = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
    if (_open_demo) {
        ImGui::ShowDemoWindow(&_open_demo);
    } else {
        if (ImGui::Begin("Battle control", &open, flags)) {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::IsMousePosValid()) {
                static float dist_from = -50.0f;
                static float dist_to = -50.0f;
                if (ImGui::BeginTabBar("Controls", tab_bar_flags)) {
                    if (ImGui::BeginTabItem("Camera")) {
                        ImGui::DragScalar("Camera x", ImGuiDataType_Float, &_camera.position.x, 0.25f, &dist_from, &dist_to, "< %.0f >");
                        ImGui::DragScalar("Camera y", ImGuiDataType_Float, &_camera.position.y, 0.25f, &dist_from, &dist_to, "< %.0f >");
                        ImGui::DragScalar("Camera z", ImGuiDataType_Float, &_camera.position.z, 0.25f, &dist_from, &dist_to, "< %.0f >");
                        ImGui::DragScalar("Camera fov", ImGuiDataType_Float, &_camera.fovy, 0.25f, &dist_from, &dist_to, "< %.0f >");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
        }
        ImGui::End();
    }
    rlImGuiEnd();
}

Camera battle::Battle::_initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                                          const float horizontalDistance, const float horizontalAngle,
                                          const float verticalAngle) {
    Camera camera;
    camera.target = cameraInitialPosition;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    //camera.position.x = horizontalDistance * std::cosh(horizontalAngle * PI / 180.0f);
    //camera.position.y = horizontalDistance * std::sinh(horizontalAngle * PI / 180.0f);
    //camera.position.z = cameraDistance * sinf(verticalAngle * PI / 180.0f);
    camera.position = Vector3{0.0f, 90.0f, 360.0f};
    SetCameraMode(camera, CAMERA_ORBITAL);
    UpdateCamera(&camera);
    return camera;
}

void battle::Battle::_render_visible() {
    BeginMode3D(_camera);
    _render_grid();
    raylib_ext::RenderInstanced(_spaceships.model.meshes[0], _spaceships.model.materials[0], _spaceships.matrices, _spaceships.colors,_spaceships.count);
    EndMode3D();
}

void battle::Battle::_render_grid() {
    DrawGrid(50, 10.0f);
}
void battle::Battle::_initialize() {
    _spaceships.model = LoadModel("assets/obj/spaceship.obj");
    //_spaceships.model = LoadModelFromMesh(GenMeshSphere(10.0f, 5, 5));
    _spaceships.shader = LoadShader("assets/shaders/glsl100/base_lighting_instanced.vs", "assets/shaders/glsl100/dream_vision.fs");
    _spaceships.shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_spaceships.shader, "mvp");
    _spaceships.shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_spaceships.shader, "viewPos");
    _spaceships.shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_spaceships.shader, "instanceTransform");
    _spaceships.shader.locs[SHADER_LOC_VERTEX_COLOR] = GetShaderLocationAttrib(_spaceships.shader, "vertexColor");

    // Ambient light level
    int ambientLoc = GetShaderLocation(_spaceships.shader, "ambient");
    const float f[4] {1.0f, 1.0f, 1.0f, 1.0f};
    SetShaderValue(_spaceships.shader, ambientLoc, f, SHADER_UNIFORM_VEC4);
    Vector3 v{150, 150, 0};
    CreateLight(LIGHT_DIRECTIONAL, v, Vector3Zero(), WHITE, _spaceships.shader);

    // NOTE: We are assigning the intancing shader to material.shader
    // to be used on mesh drawing with DrawMeshInstanced()
    Material material = LoadMaterialDefault();
    //_spaceships.model.materials[0] = material;
    material.shader = _spaceships.shader;
    //material.maps[MATERIAL_MAP_DIFFUSE].color = YELLOW;
    _spaceships.model.materials[0] = material;
    Matrix m = MatrixIdentity();
    //Vector3 v = local_to_global_coords(Vector3{x, y, z}, visible_size);
    auto translation = MatrixTranslate(0, 30, 0);
    auto rotation = MatrixRotate({1,0,0}, 0);
    auto scaling = MatrixScale(0.025, 0.025, 0.025);

    m = MatrixMultiply(m, scaling);
    m = MatrixMultiply(m, translation);
    m = MatrixMultiply(m, rotation);

    _spaceships.matrices.emplace_back(m);
    _spaceships.colors.emplace_back(Colors::col_10);
    _spaceships.count += 1;

    _texture = LoadTexture("assets/textures/spaceship.png");
    _spaceships.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _texture;

    _camera.target = Vector3{0, 30, 0};

    //_spaceships.entities.emplace_back(entity);
}
