//
// Created by geoco on 17.06.2022.
//

#ifndef TLEILAX_RAYLIB_EXTENSION_H
#define TLEILAX_RAYLIB_EXTENSION_H

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

#include <rlights.h>
#include <vector>

namespace raylib_ext {

    struct InstancedRenderData {
        float16 transform;
        Color color;
    };

    static void defineShaderMaterialUniforms(const Material &material);
    static unsigned int copyDataToVertexBuffer(const std::vector<Matrix> &transforms, const std::vector<Color> &colors, int instances);
    static void defineShaderMatrixProjection(const Material &material);
    static void drawVertexArray(const Mesh &mesh, int instances);
    static void disableTextureMaps();

    static void RenderInstanced(const Mesh &mesh, const Material &material, const std::vector<Matrix> &transforms, const std::vector<Color> &colors, const int instances) {
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

        unsigned int instancesVboId = copyDataToVertexBuffer(transforms, colors, instances);

        rlEnableShader(material.shader.id);
        rlEnableVertexArray(mesh.vaoId);

        defineShaderMaterialUniforms(material);
        defineShaderMatrixProjection(material);

        drawVertexArray(mesh, instances);

        disableTextureMaps();
        rlDisableVertexArray();
        rlDisableShader();

        rlUnloadVertexBuffer(instancesVboId);
#endif
    }

    static void disableTextureMaps() {
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
    }

    static void drawVertexArray(const Mesh &mesh, const int instances) {
        if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount * 3, 0, instances);
        else
            rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);
    }

    static void defineShaderMatrixProjection(const Material &material) {
        Matrix matModel = MatrixIdentity();
        Matrix matView = rlGetMatrixModelview();
        Matrix matModelView = MatrixIdentity();
        Matrix matProjection = rlGetMatrixProjection();

        if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
        if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

        if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
        //-----------------------------------------------------

        for (unsigned int i = 0; i < 4; i++) {
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(InstancedRenderData), (void *) (i * sizeof(Vector4)));
            rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
        }

        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, true, sizeof(InstancedRenderData), (void *) offsetof(InstancedRenderData, color));
        rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 1);

        matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

        Matrix matModelViewProjection = MatrixIdentity();
        matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);
    }

    static unsigned int copyDataToVertexBuffer(const std::vector<Matrix> &transforms, const std::vector<Color> &colors, const int instances) {
        std::vector<InstancedRenderData> instancedDataBuffer(instances);

        unsigned int instancesVboId = 0;
        for (int i = 0; i < instances; i++) {
            instancedDataBuffer[i] = InstancedRenderData{MatrixToFloatV(transforms[i]), colors[i]};
        }
        instancesVboId = rlLoadVertexBuffer(instancedDataBuffer.data(), (instances * sizeof(InstancedRenderData)), true);
        return instancesVboId;
    }

    static void defineShaderMaterialUniforms(const Material &material) {
        if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1) {
            float values[4] = {
                    (float) material.maps[MATERIAL_MAP_DIFFUSE].color.r / 255.0f,
                    (float) material.maps[MATERIAL_MAP_DIFFUSE].color.g / 255.0f,
                    (float) material.maps[MATERIAL_MAP_DIFFUSE].color.b / 255.0f,
                    (float) material.maps[MATERIAL_MAP_DIFFUSE].color.a / 255.0f};

            rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
        }

        if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1) {
            float values[4] = {
                    (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.r / 255.0f,
                    (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.g / 255.0f,
                    (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.b / 255.0f,
                    (float) material.maps[SHADER_LOC_COLOR_SPECULAR].color.a / 255.0f};

            rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
        }

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
    }
}// namespace raylib_ext
#endif//TLEILAX_RAYLIB_EXTENSION_H
