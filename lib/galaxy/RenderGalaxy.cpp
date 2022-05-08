//
// Created by geoco on 02.05.2022.
//
#include <galaxy.h>
#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>
void Galaxy::render() {
    BeginDrawing();

    ClearBackground(_core->colors.col_0);
    _render_visible();
    DrawFPS(1240, 10);
    _draw_ui();
    EndDrawing();
}

void Galaxy::_render_visible() {

    BeginMode3D(_camera);
    _entities_under_cursor.clear();
    _render_stars();
    _render_paths();
    _render_fleets();
    _render_mouse_selection();
    EndMode3D();
}

void Galaxy::_render_fleets() {
    _core->registry.view<components::Fleet, Vector3, components::Size>().each([&](const entt::entity entity, const components::Fleet &fleet, const Vector3 pos, const components::Size size) {
        Vector3 fleet_coords = local_to_global_coords(pos, _visible_size);
        fleet_coords.y += 1;
        const auto fleet_size_top = size.size / 2;
        const auto fleet_size_bottom = size.size / 4;
        BoundingBox fleet_bounds{};
        fleet_bounds.min = {fleet_coords.x - fleet_size_bottom, fleet_coords.y - (size.size) + (size.size), fleet_coords.z - fleet_size_bottom};
        fleet_bounds.max = {fleet_coords.x + fleet_size_bottom, fleet_coords.y + (size.size) + (size.size), fleet_coords.z + fleet_size_bottom};
        if (GetRayCollisionBox(GetMouseRay(GetMousePosition(), _camera), fleet_bounds).hit) {
            DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_15);
            _entities_under_cursor.emplace_back(entity);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (_camera_settings.focus_on_clicked) {
                    focus_camera(_camera, fleet_coords, 15.0f);
                }
                FleetEntity::on_click(_core->registry, entity);
                _selected_fleet = entity;
                auto *fleet_path = _core->registry.try_get<components::Path>(entity);
                if (fleet_path && !fleet_path->checkpoints.empty()) {
                    _register_path_selection(fleet_path->checkpoints);
                }
            }
        } else {
            const auto is_player_fleet = _core->registry.try_get<components::PlayerControlled>(entity);
            if (is_player_fleet) {
                DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_16);
            } else {
                const auto is_tleilaxian = _core->registry.try_get<components::Tleilaxian>(entity);
                if (!is_tleilaxian) {
                    DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_9);
                } else {
                    DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_11);
                }
            }
        }
    });
}

void Galaxy::_render_paths() {
    std::for_each(stars_paths.begin(), stars_paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
        DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), Colors::col_3);
    });
}

// Draw multiple mesh instances with material and different transforms
static void RenderInstanced(Mesh mesh, Material material, const std::vector<Matrix> &transforms, std::vector<Color> &colors, int instances) {
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Instancing required variables
    float16 *instanceTransforms = NULL;

    unsigned int instancesVboId = 0;

    // Bind shader program
    rlEnableShader(material.shader.id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.colDiffuse
    if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1) {
        float values[4] = {
                (float) material.maps[MATERIAL_MAP_DIFFUSE].color.r / 255.0f,
                (float) material.maps[MATERIAL_MAP_DIFFUSE].color.g / 255.0f,
                (float) material.maps[MATERIAL_MAP_DIFFUSE].color.b / 255.0f,
                (float) material.maps[MATERIAL_MAP_DIFFUSE].color.a / 255.0f};

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.colSpecular (if location available)
    if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1) {
        float values[4] = {
                (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.r / 255.0f,
                (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.g / 255.0f,
                (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.b / 255.0f,
                (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.a / 255.0f};

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Create instances buffer
    instanceTransforms = (float16 *) RL_MALLOC(instances * sizeof(float16));
    // Fill buffer with instances transformations as float16 arrays
    for (int i = 0; i < instances; i++) {
        instanceTransforms[i] = MatrixToFloatV(transforms[i]);
    }

    // Enable mesh VAO to attach new buffer
    rlEnableVertexArray(mesh.vaoId);

    // This could alternatively use a static VBO and either glMapBuffer() or glBufferSubData().
    // It isn't clear which would be reliably faster in all cases and on all platforms,
    // anecdotally glMapBuffer() seems very slow (syncs) while glBufferSubData() seems
    // no faster, since we're transferring all the transform matrices anyway
    instancesVboId = rlLoadVertexBuffer(instanceTransforms, (instances * sizeof(float16)) + (instances * sizeof(Color)), false);

    // Instances transformation matrices are send to shader attribute location: SHADER_LOC_MATRIX_MODEL
    for (unsigned int i = 0; i < 4; i++) {
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(Matrix), (void *) (i * sizeof(Vector4)));
        rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
    }

    rlDisableVertexBuffer();
    rlDisableVertexArray();

    // Accumulate internal matrix transform (push/pop) and view matrix
    // NOTE: In this case, model instance transformation must be computed in the shader
    matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

    // Upload model normal matrix (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++) {
        if (material.maps[i].texture.id > 0) {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
            else
                rlEnableTexture(material.maps[i].texture.id);

            rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }

    // Try binding vertex array objects (VAO)
    // or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId)) {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[1]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1) {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[2]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1) {
            /*if (mesh.vboId[3] != 0) {
                rlEnableVertexBuffer(mesh.vboId[3]);
                rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            } else {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                float value[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC2, 4);
                rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }*/
            rlEnableVertexBuffer(mesh.vboId[3]);
            rlLoadVertexBuffer(colors.data(), colors.size() * sizeof(Color), false);
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1) {
            rlEnableVertexBuffer(mesh.vboId[4]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1) {
            rlEnableVertexBuffer(mesh.vboId[5]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++) {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else {
            // Setup current eye viewport (half screen width)
            rlViewport(eye * rlGetFramebufferWidth() / 2, 0, rlGetFramebufferWidth() / 2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh instanced
        if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount * 3, 0, instances);
        else
            rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);
    }

    // Unbind all binded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++) {
        // Select current shader texture slot
        rlActiveTextureSlot(i);

        // Disable texture for active slot
        if ((i == MATERIAL_MAP_IRRADIANCE) ||
            (i == MATERIAL_MAP_PREFILTER) ||
            (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
        else
            rlDisableTexture();
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Remove instance transforms buffer
    rlUnloadVertexBuffer(instancesVboId);
    RL_FREE(instanceTransforms);
#endif
}

void Galaxy::_render_stars() {
    RenderInstanced(_star_render_instance.model.meshes[0], _star_render_instance.model.materials[0], _star_render_instance.matrices, _star_render_instance.colors,_star_render_instance.count);
    _core->registry.view<Vector3, components::Star, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        bool star_is_selected = GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), star_coords, size.size).hit;
        StarEntity::render(_core, _camera, _visible_size, entity, coords, color, size, star_is_selected);
        if (star_is_selected) {
            _entities_under_cursor.emplace_back(entity);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (_camera_settings.focus_on_clicked) {
                    focus_camera(_camera, star_coords, _camera.fovy);
                }
                _core->dispatcher.enqueue<StarSelectedEvent>(entity);
            }
            _selected_star = entity;
        }
    });
}

void StarEntity::render(const std::shared_ptr<Core> &core, const Camera &camera, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, const bool is_selected) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    //DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
    EndMode3D();

    auto *name = core->registry.try_get<components::Name>(entity);
    if (name) {
        auto name_pos = IsWindowFullscreen() ? GetWorldToScreenEx(star_coords, camera, GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())) : GetWorldToScreenEx(star_coords, camera, core->window.width, core->window.height);
        DrawRectangle(static_cast<int>(name_pos.x - 22), static_cast<int>(name_pos.y - 22), name->name.length() * 8, 12, Colors::col_0);
        DrawText(name->name.c_str(), static_cast<int>(name_pos.x - 20), static_cast<int>(name_pos.y - 20), 10, Colors::col_16);
    }

    BeginMode3D(camera);
}

void Galaxy::_render_mouse_selection() {
    if (!_entities_under_cursor.empty()) {
        auto front_entity = _entities_under_cursor.back();
        auto size = _core->registry.get<components::Size>(front_entity);
        auto position = local_to_global_coords(_core->registry.get<Vector3>(front_entity), _visible_size);
        if (_core->registry.try_get<components::Star>(front_entity)) {
            DrawSphereWires(position, size.size + 2, 6, 6, Colors::col_16);
        }
    }
}

void Galaxy::_init_star_render_instance() {
    Model m = LoadModelFromMesh(GenMeshSphere(1.0f, 6, 6));
    Shader shader = LoadShader("assets/shaders/base_lightning_instanced.vs", "assets/shaders/lighting.fs");

    _star_render_instance.shader = shader;
    // Get some shader loactions
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_star_render_instance.shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_star_render_instance.shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_star_render_instance.shader, "instanceTransform");
    shader.locs[SHADER_LOC_VERTEX_COLOR] = GetShaderLocationAttrib(_star_render_instance.shader, "vertexColor");

    // Ambient light level
    int ambientLoc = GetShaderLocation(_star_render_instance.shader, "ambient");
    const float f[4]{
            1.0f, 1.0f, 1.0f, 1.0f};
    SetShaderValue(_star_render_instance.shader, ambientLoc, f, SHADER_UNIFORM_VEC4);
    Vector3 v{50, 50, 0};
    CreateLight(LIGHT_DIRECTIONAL, v, Vector3Zero(), WHITE, _star_render_instance.shader);

    // NOTE: We are assigning the intancing shader to material.shader
    // to be used on mesh drawing with DrawMeshInstanced()
    Material material = LoadMaterialDefault();
    material.shader = _star_render_instance.shader;
    //material.maps[MATERIAL_MAP_DIFFUSE].color = YELLOW;
    m.materials[0] = material;

    _star_render_instance.model = m;
}
void Galaxy::_place_star_instance_at(float x, float y, float z, Color c, const Vector3 size) {
    Matrix m = MatrixIdentity();
    Vector3 v = local_to_global_coords(Vector3{x, y, z}, size);
    auto translation = MatrixTranslate(v.x, v.y, v.z);
    auto rotation = MatrixRotate({1,0,0}, 0);
    auto scale = MatrixScale(1.0f, 1.0f, 1.0f);

    m = MatrixMultiply(m, translation);
    m = MatrixMultiply(m, rotation);
    m = MatrixMultiply(m, scale);

    _star_render_instance.matrices.emplace_back(m);
    _star_render_instance.colors.emplace_back(c);
    _star_render_instance.count += 1;
}
