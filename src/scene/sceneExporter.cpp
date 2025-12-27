//
// Created by ChatGPT on 26-01-08.
//

#include <scene/sceneExporter.h>

#include <assets/model.h>
#include <filesystem>
#include <fstream>
#include <graphics/mesh.h>
#include <iomanip>
#include <scene/entity.h>
#include <scene/scene.h>
#include <scene/transform.h>
#include <utils/common.h>

#include <cmath>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp>

namespace
{
glm::vec3 safeNormalize(const glm::vec3& n)
{
    float len2 = glm::length2(n);
    if (len2 < 1e-12f) return {0, 1, 0};
    return n / std::sqrt(len2);
}
} // namespace

bool exportSceneToOBJ(const Scene& scene, const std::string& outputPath)
{
    std::filesystem::path outPath(outputPath);
    const auto parent = outPath.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec)
            LOG(WARNING) << "[SceneExporter] Failed to ensure directory: " << parent << ", " << ec.message();
    }

    std::ofstream out(outPath);
    if (!out.is_open())
    {
        LOG(ERROR) << "[SceneExporter] Failed to open file for writing: " << outPath;
        return false;
    }

    out << "# Exported by Simple 3D Modeling\n";

    size_t vOffset = 0;
    size_t vtOffset = 0;
    size_t vnOffset = 0;
    size_t exportedMeshes = 0;

    for (const auto& ePtr : scene.getEntities())
    {
        const Entity* entity = ePtr.get();
        if (!entity || !entity->isVisible()) continue;

        const auto& modelPtr = entity->getModel();
        if (!modelPtr || modelPtr->submeshes.empty()) continue;

        const glm::mat4 entityModel = entity->getTransform().modelMatrix();

        for (const auto& sm : modelPtr->submeshes)
        {
            if (!sm.mesh) continue;

            const auto& verts = sm.mesh->getCpuVertices();
            if (verts.empty())
            {
                LOG(WARNING) << "[SceneExporter] Skip submesh without CPU vertices: " << sm.name;
                continue;
            }
            const auto& indices = sm.mesh->getCpuIndices();

            const glm::mat4 modelMat = entityModel * sm.local;
            glm::mat3 normalMat(1.0f);
            const glm::mat3 linear = glm::mat3(modelMat);
            const float det = glm::determinant(linear);
            if (std::abs(det) > 1e-8f)
                normalMat = glm::transpose(glm::inverse(linear));

            std::string objName = entity->getName();
            if (!sm.name.empty())
            {
                if (!objName.empty())
                    objName += "_";
                objName += sm.name;
            }
            if (objName.empty())
                objName = "object_" + std::to_string(exportedMeshes);

            out << "o " << objName << "\n";
            out << std::fixed << std::setprecision(6);

            for (const auto& v : verts)
            {
                glm::vec3 pos = glm::vec3(modelMat * glm::vec4(v.position, 1.0f));
                out << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
            }
            for (const auto& v : verts)
            {
                out << "vt " << v.uv.x << " " << v.uv.y << "\n";
            }
            for (const auto& v : verts)
            {
                glm::vec3 n = safeNormalize(normalMat * v.normal);
                out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
            }

            auto emitFace = [&](std::uint32_t i0, std::uint32_t i1, std::uint32_t i2)
            {
                out << "f "
                    << vOffset + static_cast<size_t>(i0) + 1 << "/" << vtOffset + static_cast<size_t>(i0) + 1 << "/" << vnOffset + static_cast<size_t>(i0) + 1 << " "
                    << vOffset + static_cast<size_t>(i1) + 1 << "/" << vtOffset + static_cast<size_t>(i1) + 1 << "/" << vnOffset + static_cast<size_t>(i1) + 1 << " "
                    << vOffset + static_cast<size_t>(i2) + 1 << "/" << vtOffset + static_cast<size_t>(i2) + 1 << "/" << vnOffset + static_cast<size_t>(i2) + 1 << "\n";
            };

            if (!indices.empty())
            {
                if (indices.size() % 3 != 0)
                    LOG(WARNING) << "[SceneExporter] Index count not divisible by 3 in submesh: " << sm.name;
                for (size_t i = 0; i + 2 < indices.size(); i += 3)
                {
                    emitFace(indices[i], indices[i + 1], indices[i + 2]);
                }
            }
            else
            {
                if (verts.size() % 3 != 0)
                    LOG(WARNING) << "[SceneExporter] Vertex count not divisible by 3 in non-indexed submesh: " << sm.name;
                for (size_t i = 0; i + 2 < verts.size(); i += 3)
                {
                    emitFace(static_cast<std::uint32_t>(i),
                             static_cast<std::uint32_t>(i + 1),
                             static_cast<std::uint32_t>(i + 2));
                }
            }

            vOffset += verts.size();
            vtOffset += verts.size();
            vnOffset += verts.size();
            ++exportedMeshes;
            out << "\n";
        }
    }

    if (exportedMeshes == 0)
        LOG(WARNING) << "[SceneExporter] No mesh data exported to OBJ (empty scene?)";
    else
        LOG(INFO) << "[SceneExporter] Exported " << exportedMeshes << " mesh parts to " << outPath;

    return true;
}
