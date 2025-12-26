//
// Created by Remilia on 25-12-18.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <utils/common.h>
#include <filesystem>
#include <memory>
#include <vector>

class Scene;
class Shader;
class Mesh;
class Material;
class DirectionalLight;
struct DrawCmd;

class Renderer
{
public:
    struct FrameParams
    {
        glm::vec4 clearColor{0.1f, 0.1f, 0.15f, 1.0f};
        bool clearColorBuffer = true;
        bool clearDepthBuffer = true;

        bool enableDepthTest   = true;
        bool enableCullFace    = false;
        bool enableBlend       = false;   // 基础状态；透明物体会在 renderScene 内临时开启
        bool enableMultisample = true;
    };

public:
    Renderer() = default;
    ~Renderer();

    // 每帧每窗口调用一次
    void beginFrame(GLFWwindow* window, const FrameParams& params);
    void endFrame();

    // 直接渲染 Scene（Scene 管 lights；Material 持 Shader）
    void renderScene(const Scene& scene,
                     const glm::mat4& view,
                     const glm::mat4& proj,
                     const glm::vec3& viewPos);

    void saveScreenshot(const std::filesystem::path& path);

private:
    // --- shadow mapping ---
    struct ShadowContext
    {
        bool enabled = false;
        glm::mat4 lightSpace{1.0f};
        GLuint shadowMap = 0;
        float bias = 0.0015f;
    };

    struct ShadowResources
    {
        GLuint fbo = 0;
        GLuint map = 0;
        glm::mat4 lightSpace{1.0f};

        bool available() const { return fbo != 0 && map != 0; }
    };

    // --- frame ---
    static void applyViewport(GLFWwindow* window);

    static void applyFrameState(const FrameParams& params);

    static void clearBuffers(const FrameParams& params);

    // --- gl state helpers (避免频繁切换) ---
    void setCull(bool enable);
    void setBlend(bool enable);
    void setDepthWrite(bool enable);

    // --- cache binds ---
    void bindProgram(GLuint program);
    void bindTexture(GLuint unit, GLuint tex);

    // --- uploads (Scheme A: only Shader::setXxx) ---
    void uploadGlobals(Shader& shader,
                       const Scene& scene,
                       const glm::mat4& view,
                       const glm::mat4& proj,
                       const glm::vec3& viewPos,
                       const ShadowContext& shadow);

    void uploadMaterial(Shader& shader, const Material& mat);

    // --- draw ---
    static void drawMesh(const Mesh& mesh);

private:
    bool ensureShadowResources();
    bool ensureShadowShader();
    bool renderShadowMap(const std::vector<struct DrawCmd>& commands, const glm::mat4& lightSpace);
    glm::mat4 buildDirectionalLightMatrix(const DirectionalLight& light, const glm::vec3& focus) const;
    void destroyShadowResources();

    GLFWwindow* currentWindow = nullptr;

    // binding cache
    GLuint boundProgram = 0;
    std::array<GLuint, 8> boundTextures{0,0,0,0,0,0,0,0};

    // state cache
    bool baseCullEnabled  = true;
    bool baseBlendEnabled = false;
    bool baseDepthWrite   = true;

    bool curCullEnabled   = true;
    bool curBlendEnabled  = false;
    bool curDepthWrite    = true;

    static constexpr int MAX_POINT_LIGHTS = 16;

    // shadow config/state
    ShadowResources shadowResources{};
    std::unique_ptr<Shader> shadowShader;
    int shadowMapSize = 4096;
    float shadowOrthoSize = 12.0f;
    float shadowNearPlane = 5.0f;
    float shadowFarPlane = 50.0f;
    float shadowBias = 0.0005f;
};


#endif //RENDERER_H
