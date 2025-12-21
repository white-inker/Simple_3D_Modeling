//
// Created by Remilia on 25-12-18.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <utils/common.h>
#include <string>
#include <unordered_map>
#include <vector>

class Scene;
class Shader;
class Mesh;
class Material;

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

    // 每帧每窗口调用一次
    void beginFrame(GLFWwindow* window, const FrameParams& params);
    void endFrame();

    // 直接渲染 Scene（Scene 管 lights；Material 持 Shader）
    void renderScene(const Scene& scene,
                     const glm::mat4& view,
                     const glm::mat4& proj,
                     const glm::vec3& viewPos);

private:
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
    static void uploadGlobals(Shader& shader,
                              const Scene& scene,
                              const glm::mat4& view,
                              const glm::mat4& proj,
                              const glm::vec3& viewPos);

    void uploadMaterial(Shader& shader, const Material& mat);

    // --- draw ---
    static void drawMesh(const Mesh& mesh);

private:
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
};


#endif //RENDERER_H
