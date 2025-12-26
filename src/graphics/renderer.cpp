//
// Created by Remilia on 25-12-18.
//
#include "graphics/Renderer.h"

#include <algorithm>
#include <glm/gtc/matrix_inverse.hpp>
#include <third_party/stb_image_write.h>

#include <scene/scene.h>
#include <scene/entity.h>
#include <scene/lighting.h>
#include <graphics/shader.h>
#include <graphics/mesh.h>
#include <graphics/material.h>
#include <scene/transform.h>
#include <assets/model.h>

struct DrawCmd
{
    GLuint program = 0;
    Shader* shader = nullptr;
    const Mesh* mesh = nullptr;
    const Material* mat = nullptr;

    glm::mat4 model{1.0f};

    // for sorting
    GLuint tBase = 0;
    GLuint tNormal = 0;
    GLuint tMR = 0;
    GLuint tAO = 0;
    GLuint tEmissive = 0;

    bool doubleSided = false;
    bool alphaBlend  = false;

    float depthKey = 0.0f; // for transparent sorting (far->near)
};

static bool isCmdValid(const DrawCmd& c)
{
    return c.shader && c.mesh && c.mat && c.program != 0;
}

// ----------- Renderer impl -----------

void Renderer::beginFrame(GLFWwindow* window, const FrameParams& params)
{
    currentWindow = window;
    if (window) glfwMakeContextCurrent(window);

    glEnable(GL_FRAMEBUFFER_SRGB);

    boundProgram = 0;
    boundTextures.fill(0);

    applyViewport(window);
    applyFrameState(params);
    clearBuffers(params);

    baseCullEnabled  = params.enableCullFace;
    baseBlendEnabled = params.enableBlend;
    baseDepthWrite   = true;

    curCullEnabled   = baseCullEnabled;
    curBlendEnabled  = baseBlendEnabled;
    curDepthWrite    = baseDepthWrite;
}

void Renderer::endFrame()
{
    glDisable(GL_FRAMEBUFFER_SRGB);
    glBindVertexArray(0);
    glUseProgram(0);
    boundProgram = 0;
}

void Renderer::renderScene(const Scene& scene,
                           const glm::mat4& view,
                           const glm::mat4& proj,
                           const glm::vec3& viewPos)
{
    std::vector<DrawCmd> opaque;
    std::vector<DrawCmd> transparent;

    const auto& entities = scene.getEntities();

    // 预估容量
    opaque.reserve(entities.size() * 2);
    transparent.reserve(entities.size());

    // 1) collect commands
    for (const auto& ePtr : entities)
    {
        const Entity* e = ePtr.get();
        if (!e || !e->isRenderable()) continue;

        const auto& modelPtr = e->getModel();     // std::shared_ptr<Model>
        if (!modelPtr) continue;

        const auto& entityMat = e->getMaterial(); // Material&（当前：一个 entity 一个材质）
        //if (!entityMat.isValid()) continue;

        const glm::mat4 entityModel = e->getTransform().modelMatrix();

        // 一个 entity -> 多个 submesh draw
        for (const auto& sm : modelPtr->submeshes)
        {
            if (!sm.mesh) continue;

            const auto& mat = sm.material ? *sm.material : entityMat;
            if (!mat.isValid()) continue;

            Shader* shader = mat.shader.get();
            if (!shader) continue;

            DrawCmd c;
            c.program = shader->id();
            c.shader  = shader;
            c.mesh    = sm.mesh.get();
            c.mat     = &mat;

            // 如果 Submesh 有 local 变换：entityModel * sm.local
            // 如果没有 sm.local 字段，就把这一行改为：c.model = entityModel;
            c.model = entityModel * sm.local;

            // 纹理 key
            c.tBase     = mat.baseColorId();
            c.tNormal   = mat.normalId();
            c.tMR       = mat.mrId();
            c.tAO       = mat.aoId();
            c.tEmissive = mat.emissiveId();

            c.doubleSided = mat.doubleSided;
            c.alphaBlend  = mat.alphaBlend;

            // depth key：用模型矩阵平移列估一个距离（透明排序用）
            const glm::vec3 worldPos = glm::vec3(c.model[3]);
            c.depthKey = glm::length(worldPos - viewPos);

            (c.alphaBlend ? transparent : opaque).push_back(c);
        }
    }

    if (opaque.empty() && transparent.empty())
        return;

    // 2) sort opaque to minimize state changes
    std::sort(opaque.begin(), opaque.end(), [](const DrawCmd& a, const DrawCmd& b) {
        if (a.program != b.program) return a.program < b.program;

        if (a.tBase != b.tBase) return a.tBase < b.tBase;
        if (a.tNormal != b.tNormal) return a.tNormal < b.tNormal;
        if (a.tMR != b.tMR) return a.tMR < b.tMR;
        if (a.tAO != b.tAO) return a.tAO < b.tAO;
        if (a.tEmissive != b.tEmissive) return a.tEmissive < b.tEmissive;

        if (a.doubleSided != b.doubleSided) return a.doubleSided < b.doubleSided;
        return false;
    });

    // 3) sort transparent back-to-front (far -> near)
    std::sort(transparent.begin(), transparent.end(), [](const DrawCmd& a, const DrawCmd& b) {
        return a.depthKey > b.depthKey;
    });

    // ---------- draw opaque pass ----------
    setBlend(false);
    setDepthWrite(true);

    GLuint lastProgram = 0;
    for (const auto& c : opaque)
    {
        if (!isCmdValid(c)) continue;

        if (c.program != lastProgram)
        {
            bindProgram(c.program);
            //c.shader->bind();
            uploadGlobals(*c.shader, scene, view, proj, viewPos);
            lastProgram = c.program;
        }

        setCull(baseCullEnabled && !c.doubleSided);

        c.shader->setMat4("Model", c.model);
        glm::mat3 normalMat = glm::inverseTranspose(glm::mat3(c.model));
        c.shader->setMat3("NormalMatrix", normalMat);

        uploadMaterial(*c.shader, *c.mat);
        drawMesh(*c.mesh);
    }

    // ---------- draw transparent pass ----------
    if (!transparent.empty())
    {
        setBlend(true);
        setDepthWrite(false);

        lastProgram = 0;
        for (const auto& c : transparent)
        {
            if (!isCmdValid(c)) continue;

            if (c.program != lastProgram)
            {
                bindProgram(c.program);
                //c.shader->bind();
                uploadGlobals(*c.shader, scene, view, proj, viewPos);
                lastProgram = c.program;
            }

            setCull(baseCullEnabled && !c.doubleSided);

            c.shader->setMat4("Model", c.model);
            glm::mat3 normalMat = glm::inverseTranspose(glm::mat3(c.model));
            c.shader->setMat3("NormalMatrix", normalMat);

            uploadMaterial(*c.shader, *c.mat);
            drawMesh(*c.mesh);
        }

        setDepthWrite(true);
        setBlend(baseBlendEnabled);
        setCull(baseCullEnabled);
    }
    else
    {
        setBlend(baseBlendEnabled);
        setCull(baseCullEnabled);
        setDepthWrite(true);
    }
}

void Renderer::saveScreenshot(const std::filesystem::path& path)
{
    if (!currentWindow)
    {
        LOG(WARNING) << "Renderer::saveScreenshot called with no active window";
        return;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(currentWindow, &width, &height);
    if (width <= 0 || height <= 0)
    {
        LOG(WARNING) << "Renderer::saveScreenshot skipped because framebuffer size is invalid";
        return;
    }

    std::vector<unsigned char> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(1);
    const int stride = width * 4;
    if (stbi_write_png(path.string().c_str(), width, height, 4, pixels.data(), stride) == 0)
    {
        LOG(ERROR) << "Failed to write screenshot to " << path;
    }
    else
    {
        LOG(INFO) << "Saved screenshot to " << path;
    }
}

// ---------------- frame helpers ----------------

void Renderer::applyViewport(GLFWwindow* window)
{
    if (!window) return;

    int w = 0, h = 0;
    glfwGetFramebufferSize(window, &w, &h);
    h = std::max(1, h);
    glViewport(0, 0, w, h);
}

void Renderer::applyFrameState(const FrameParams& params)
{
    if (params.enableDepthTest) glEnable(GL_DEPTH_TEST);
    else                       glDisable(GL_DEPTH_TEST);

    if (params.enableCullFace)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    if (params.enableBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (params.enableMultisample) glEnable(GL_MULTISAMPLE);
    else                          glDisable(GL_MULTISAMPLE);

    glDepthMask(GL_TRUE);
}

void Renderer::clearBuffers(const FrameParams& params)
{
    GLbitfield mask = 0;

    if (params.clearColorBuffer)
    {
        glClearColor(params.clearColor.r, params.clearColor.g, params.clearColor.b, params.clearColor.a);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (params.clearDepthBuffer)
    {
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (mask) glClear(mask);
}

// ---------------- state cache ----------------

void Renderer::setCull(bool enable)
{
    if (curCullEnabled == enable) return;
    curCullEnabled = enable;
    if (enable) glEnable(GL_CULL_FACE);
    else        glDisable(GL_CULL_FACE);
}

void Renderer::setBlend(bool enable)
{
    if (curBlendEnabled == enable) return;
    curBlendEnabled = enable;
    if (enable)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void Renderer::setDepthWrite(bool enable)
{
    if (curDepthWrite == enable) return;
    curDepthWrite = enable;
    glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

// ---------------- bind cache ----------------

void Renderer::bindProgram(const GLuint program)
{
    if (boundProgram == program) return;
    glUseProgram(program);
    boundProgram = program;
}

void Renderer::bindTexture(const GLuint unit, const GLuint tex)
{
    if (unit >= boundTextures.size()) return;
    if (boundTextures[unit] == tex) return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    boundTextures[unit] = tex;
}

// ---------------- uploads ----------------

void Renderer::uploadGlobals(Shader& shader,
                             const Scene& scene,
                             const glm::mat4& view,
                             const glm::mat4& proj,
                             const glm::vec3& viewPos)
{
    shader.setMat4("View", view);
    shader.setMat4("Proj", proj);

    shader.setVec3("ViewPos", viewPos);

    const auto& L = scene.getLighting();
    shader.setVec3("Ambient", L.ambient);

    if (const DirectionalLight* d = L.directional(); d && d->enabled)
    {
        shader.setVec3("dirLight.direction", d->direction);
        shader.setVec3("dirLight.color", d->color);
        shader.setFloat("dirLight.intensity", d->intensity);
    }
    else
    {
        shader.setVec3("dirLight.direction", glm::vec3(0, -1, 0));
        shader.setVec3("dirLight.color", glm::vec3(1, 1, 1));
        shader.setFloat("dirLight.intensity", 0.0f);
    }

    int count = 0;
    const auto& pts = L.pointLights();
    for (const auto& pPtr : pts)
    {
        if (!pPtr || !pPtr->enabled) continue;
        if (count >= MAX_POINT_LIGHTS) break;

        const PointLight& p = *pPtr;
        const std::string base = "PointLights[" + std::to_string(count) + "].";
        shader.setVec3(base + "position", p.position);
        shader.setVec3(base + "color", p.color);
        shader.setFloat(base + "intensity", p.intensity);
        shader.setFloat(base + "range", p.range);

        ++count;
    }
    shader.setInt("PointLightCount", count);
}

void Renderer::uploadMaterial(Shader& shader, const Material& mat)
{
    shader.setVec4("Material_BaseColorFactor", mat.baseColorFactor);
    shader.setFloat("Material_MetallicFactor", mat.metallicFactor);
    shader.setFloat("Material_RoughnessFactor", mat.roughnessFactor);

    shader.setFloat("Material_NormalScale", mat.normalScale);
    shader.setFloat("Material_OcclusionStrength", mat.occlusionStrength);

    shader.setVec3("Material_EmissiveFactor", mat.emissiveFactor);
    shader.setFloat("Material_EmissiveStrength", mat.emissiveStrength);

    shader.setInt("Material_UseBaseColorMap", mat.baseColorId() ? 1 : 0);
    shader.setInt("Material_UseNormalMap", mat.normalId() ? 1 : 0);
    shader.setInt("Material_UseMetallicRoughnessMap", mat.mrId() ? 1 : 0);
    shader.setInt("Material_UseOcclusionMap", mat.aoId() ? 1 : 0);
    shader.setInt("Material_UseEmissiveMap", mat.emissiveId() ? 1 : 0);

    shader.setInt("Material_DoubleSided", mat.doubleSided ? 1 : 0);

    if (mat.baseColorId())
    {
        bindTexture(0, mat.baseColorId());
        shader.setInt("BaseColorMap", 0);
    }
    if (mat.normalId())
    {
        bindTexture(1, mat.normalId());
        shader.setInt("NormalMap", 1);
    }
    if (mat.mrId())
    {
        bindTexture(2, mat.mrId());
        shader.setInt("MetallicRoughnessMap", 2);
    }
    if (mat.aoId())
    {
        bindTexture(3, mat.aoId());
        shader.setInt("OcclusionMap", 3);
    }
    if (mat.emissiveId())
    {
        bindTexture(4, mat.emissiveId());
        shader.setInt("EmissiveMap", 4);
    }
}

// ---------------- draw ----------------

void Renderer::drawMesh(const Mesh& mesh)
{
    glBindVertexArray(mesh.getVao());

    if (mesh.getIndexCount() > 0)
        glDrawElements(GL_TRIANGLES, mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());

    glBindVertexArray(0);
}
