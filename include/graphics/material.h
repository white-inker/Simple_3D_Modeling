//
// Created by Remilia on 25-12-18.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include <utils/common.h>
#include <graphics/shader.h>
#include <graphics/texture.h>

class Material
{
public:
    std::shared_ptr<Shader> shader;

    // ---------------- PBR Parameters (metallic-roughness) ----------------
    // Base color (albedo) in linear workflow:
    // - 如果你的贴图是 sRGB，需要在纹理创建时用 GL_SRGB8_ALPHA8 或在 shader 里做 gamma
    glm::vec4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};

    // Metallic [0,1], Roughness [0,1]
    float metallicFactor  = 0.0f;
    float roughnessFactor = 0.5f;

    // Normal map intensity
    float normalScale = 1.0f;

    // Ambient occlusion 强度
    float occlusionStrength = 1.0f;

    // Emissive（自发光，常用于灯带/屏幕）
    glm::vec3 emissiveFactor{0.0f, 0.0f, 0.0f};
    float emissiveStrength = 1.0f;

    // ---------------- Textures (0 means none) ----------------
    std::shared_ptr<Texture2D> baseColorMap;          // slot 0
    std::shared_ptr<Texture2D> normalMap;             // slot 1
    std::shared_ptr<Texture2D> metallicRoughnessMap;  // slot 2 (G=roughness, B=metallic)
    std::shared_ptr<Texture2D> occlusionMap;          // slot 3 (R=AO)
    std::shared_ptr<Texture2D> emissiveMap;           // slot 4

    // ---------------- Render State hints (optional but useful) ----------------
    // 透明
    bool alphaBlend = false;

    // 双面 窗帘/薄片
    bool doubleSided = false;

    // ---------------- helpers ----------------
    bool isValid() const { return shader && (*shader); }
    GLuint program() const { return shader ? shader->id() : 0; }

    bool hasBaseColorMap() const { return baseColorMap->id() != 0; }
    bool hasNormalMap() const { return normalMap->id() != 0; }
    bool hasMetallicRoughnessMap() const { return metallicRoughnessMap->id() != 0; }
    bool hasOcclusionMap() const { return occlusionMap->id() != 0; }
    bool hasEmissiveMap() const { return emissiveMap->id() != 0; }

    GLuint baseColorId() const { return baseColorMap ? baseColorMap->id() : 0; }
    GLuint normalId() const { return normalMap ? normalMap->id() : 0; }
    GLuint mrId() const { return metallicRoughnessMap ? metallicRoughnessMap->id() : 0; }
    GLuint aoId() const { return occlusionMap ? occlusionMap->id() : 0; }
    GLuint emissiveId() const { return emissiveMap ? emissiveMap->id() : 0; }

    void clampParams()
    {
        metallicFactor  = glm::clamp(metallicFactor,  0.0f, 1.0f);
        roughnessFactor = glm::clamp(roughnessFactor, 0.04f, 1.0f);
        normalScale = glm::max(normalScale, 0.0f);
        occlusionStrength = glm::clamp(occlusionStrength, 0.0f, 1.0f);
        emissiveStrength  = glm::max(emissiveStrength, 0.0f);
        baseColorFactor = glm::clamp(baseColorFactor, 0.0f, 1.0f);
        emissiveFactor  = glm::max(emissiveFactor, glm::vec3(0.0f));
    }
};


#endif //MATERIAL_H
