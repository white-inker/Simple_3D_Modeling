
#include <assets/assetManager.h>

#include <filesystem>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <numeric>
#include <optional>

#include <graphics/mesh.h>
#include <graphics/material.h>

// tinyobjloader
#include <third_party/tiny_obj_loader.h>
#include <third_party/tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

AssetManager::AssetManager(std::string assetRootDir)
    : m_root(std::move(assetRootDir))
{
}

void AssetManager::setRootDir(const std::string& root) { m_root = root; }
const std::string& AssetManager::rootDir() const { return m_root; }

std::string AssetManager::makeAbsPath(const std::string& relativePath) const
{
    const std::filesystem::path p = std::filesystem::path(m_root) / relativePath;
    return p.lexically_normal().string();
}

std::string AssetManager::makeTextureKey(const std::string& absPath, const TextureParams& params)
{
    std::ostringstream ss;
    ss << absPath
       << "|cs=" << (params.colorSpace == ColorSpace::SRGB ? 1 : 0)
       << "|flip=" << (params.flipY ? 1 : 0)
       << "|mip=" << (params.generateMipmaps ? 1 : 0)
       << "|wrapS=" << params.wrapS
       << "|wrapT=" << params.wrapT
       << "|min=" << params.minFilter
       << "|mag=" << params.magFilter
       << "|aniso=" << params.anisotropy;
    if (params.forceChannels.has_value()) ss << "|ch=" << *params.forceChannels;
    return ss.str();
}

std::shared_ptr<Texture2D> AssetManager::loadTexture2D(const std::string& relativePath,
                                                       const TextureParams& params,
                                                       bool forceReload)
{
    const std::string abs = makeAbsPath(relativePath);
    const std::string key = makeTextureKey(abs, params);

    if (!forceReload)
    {
        const auto it = m_textureCache.find(key);
        if (it != m_textureCache.end())
        {
            if (auto sp = it->second.lock())
                return sp;
        }
    }

    auto tex = std::make_shared<Texture2D>();
    if (!tex->loadFromFile(abs, params))
        return nullptr;

    m_textureCache[key] = tex;
    return tex;
}

// ---------------- OBJ loading with tinyobjloader ----------------

struct IndexKey
{
    int v = -1;
    int vt = -1;
    int vn = -1;

    bool operator==(const IndexKey& o) const { return v==o.v && vt==o.vt && vn==o.vn; }
};

struct IndexKeyHash
{
    std::size_t operator()(const IndexKey& k) const noexcept
    {
        // hash combine
        std::size_t h = 1469598103934665603ull;
        auto mix = [&](int x){
            h ^= static_cast<std::size_t>(x + 0x9e3779b9);
            h *= 1099511628211ull;
        };
        mix(k.v); mix(k.vt); mix(k.vn);
        return h;
    }
};

static glm::vec3 fetchVec3(const std::vector<float>& a, int idx)
{
    return {a[3*idx + 0], a[3*idx + 1], a[3*idx + 2]};
}
static glm::vec2 fetchVec2(const std::vector<float>& a, int idx)
{
    return {a[2*idx + 0], a[2*idx + 1]};
}
static glm::vec3 computeFaceNormal(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
{
    glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
    float len = glm::length(n);
    if (len < 1e-8f) return {0,1,0};
    return n / len;
}

std::shared_ptr<Model> AssetManager::loadModelOBJ(const std::string& relativePath, bool forceReload)
{
    const std::string abs = makeAbsPath(relativePath);

    if (!forceReload)
    {
        auto it = m_modelCache.find(abs);
        if (it != m_modelCache.end())
        {
            if (auto sp = it->second.lock())
                return sp;
        }
    }

    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;
    cfg.vertex_color = false;

    // 让 tinyobj 可以找到 .mtl（同目录）
    cfg.mtl_search_path = std::filesystem::path(abs).parent_path().string();

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(abs, cfg))
    {
        LOG(ERROR) << "[AssetManager][OBJ] load failed: " << abs ;
        if (!reader.Error().empty()) LOG(ERROR) << reader.Error();
        return nullptr;
    }
    if (!reader.Warning().empty())
        LOG(ERROR) << "[AssetManager][OBJ] warning: " << reader.Warning() ;

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    // const auto& materials = reader.GetMaterials(); // 解析 mtl

    auto model = std::make_shared<Model>();
    model->sourcePath = abs;

    // 每个 shape 可能包含多个 material id，我们按 material 分 submesh
    for (const auto& shape : shapes)
    {
        // material_id -> builder
        struct Builder
        {
            std::vector<Mesh::Vertex> verts;
            std::vector<std::uint32_t> indices;
            std::unordered_map<IndexKey, std::uint32_t, IndexKeyHash> dedup;
        };

        std::unordered_map<int, Builder> builders; // key: material id (可能为 -1)
        builders.reserve(8);

        size_t indexOffset = 0;
        const auto& faceVertCounts = shape.mesh.num_face_vertices;
        const auto& matIds = shape.mesh.material_ids;
        const auto& idxs = shape.mesh.indices;

        for (size_t f = 0; f < faceVertCounts.size(); ++f)
        {
            const int fv = faceVertCounts[f];  // triangulate 后 fv 应为 3
            const int matId = (f < matIds.size() ? matIds[f] : -1);

            Builder& b = builders[matId];

            // 取这个 face 的三个点，用于计算缺失法线
            glm::vec3 p[3]{};
            bool hasAllPos = true;

            for (int v = 0; v < fv; ++v)
            {
                const tinyobj::index_t& id = idxs[indexOffset + v];
                if (id.vertex_index < 0) { hasAllPos = false; continue; }
                p[v] = fetchVec3(attrib.vertices, id.vertex_index);
            }

            glm::vec3 faceN = (fv >= 3 && hasAllPos) ? computeFaceNormal(p[0], p[1], p[2]) : glm::vec3(0,1,0);

            for (int v = 0; v < fv; ++v)
            {
                const tinyobj::index_t& id = idxs[indexOffset + v];

                IndexKey key{ id.vertex_index, id.texcoord_index, id.normal_index };

                auto it = b.dedup.find(key);
                if (it != b.dedup.end())
                {
                    b.indices.push_back(it->second);
                    continue;
                }

                Mesh::Vertex vert;
                // position
                if (id.vertex_index >= 0)
                    vert.position = fetchVec3(attrib.vertices, id.vertex_index);

                // uv
                if (id.texcoord_index >= 0)
                {
                    vert.uv = fetchVec2(attrib.texcoords, id.texcoord_index);
                    // OBJ 的 vt 通常是左下原点；如果你加载纹理时 flipY=true，这里不用再翻
                    // 如果你 flipY=false，可能需要：vert.uv.y = 1.0f - vert.uv.y;
                }

                // normal
                if (id.normal_index >= 0)
                    vert.normal = fetchVec3(attrib.normals, id.normal_index);
                else
                    vert.normal = faceN; // 缺法线：先用面法线

                auto newId = static_cast<std::uint32_t>(b.verts.size());
                b.verts.push_back(vert);
                b.dedup.emplace(key, newId);
                b.indices.push_back(newId);
            }

            indexOffset += fv;
        }

        // builders -> submeshes
        int smIndex = 0;
        for (auto& [matId, b] : builders)
        {
            if (b.verts.empty()) continue;

            auto mesh = std::make_shared<Mesh>(b.verts, b.indices);

            Model::Submesh sm;
            sm.name = shape.name.empty() ? ("submesh_" + std::to_string(smIndex)) : (shape.name + "_" + std::to_string(smIndex));
            sm.mesh = mesh;
            sm.material = nullptr;
            sm.local = glm::mat4(1.0f);

            model->submeshes.push_back(std::move(sm));
            ++smIndex;
        }
    }

    m_modelCache[abs] = model;
    return model;
}

// ---------------- GLB loading with tinygltf ----------------

static glm::mat4 composeNodeTransform(const tinygltf::Node& node)
{
    if (node.matrix.size() == 16)
    {
        glm::mat4 m(1.0f);
        // glTF: column-major, GLM: m[col][row]
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                m[col][row] = static_cast<float>(node.matrix[col * 4 + row]);
            }
        }
        return m;
    }

    glm::vec3 translation(0.0f);
    if (node.translation.size() == 3)
        translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);

    glm::vec3 scale(1.0f);
    if (node.scale.size() == 3)
        scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

    glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
    if (node.rotation.size() == 4)
        rotation = glm::quat(static_cast<float>(node.rotation[3]),
                             static_cast<float>(node.rotation[0]),
                             static_cast<float>(node.rotation[1]),
                             static_cast<float>(node.rotation[2]));

    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 R = glm::toMat4(rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    return T * R * S;
}

static size_t accessorStride(const tinygltf::Accessor& accessor, const tinygltf::BufferView& view)
{
    size_t stride = accessor.ByteStride(view);
    if (stride == 0)
        stride = tinygltf::GetNumComponentsInType(accessor.type) *
                 tinygltf::GetComponentSizeInBytes(accessor.componentType);
    return stride;
}

static bool readAccessorVec3(const tinygltf::Model& model,
                             int accessorIndex,
                             std::vector<glm::vec3>& out)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
        return false;
    const auto& accessor = model.accessors[accessorIndex];
    if (accessor.type != TINYGLTF_TYPE_VEC3 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        return false;
    if (accessor.bufferView < 0) return false;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];

    const size_t stride = accessorStride(accessor, view);
    const unsigned char* dataPtr = buffer.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i)
    {
        const float* f = reinterpret_cast<const float*>(dataPtr + i * stride);
        out[i] = { f[0], f[1], f[2] };
    }
    return true;
}

static bool readAccessorVec2(const tinygltf::Model& model,
                             int accessorIndex,
                             std::vector<glm::vec2>& out)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
        return false;
    const auto& accessor = model.accessors[accessorIndex];
    if (accessor.type != TINYGLTF_TYPE_VEC2 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
        return false;
    if (accessor.bufferView < 0) return false;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];

    const size_t stride = accessorStride(accessor, view);
    const unsigned char* dataPtr = buffer.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i)
    {
        const float* f = reinterpret_cast<const float*>(dataPtr + i * stride);
        out[i] = { f[0], f[1] };
    }
    return true;
}

static bool readIndices(const tinygltf::Model& model,
                        int accessorIndex,
                        std::vector<std::uint32_t>& out)
{
    if (accessorIndex < 0)
        return false;
    if (accessorIndex >= static_cast<int>(model.accessors.size()))
        return false;

    const auto& accessor = model.accessors[accessorIndex];
    if (accessor.bufferView < 0) return false;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];

    const size_t stride = accessorStride(accessor, view);
    const unsigned char* dataPtr = buffer.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i)
    {
        const unsigned char* p = dataPtr + i * stride;
        switch (accessor.componentType)
        {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                out[i] = *reinterpret_cast<const std::uint32_t*>(p); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                out[i] = *reinterpret_cast<const std::uint16_t*>(p); break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                out[i] = *reinterpret_cast<const std::uint8_t*>(p); break;
            default:
                return false;
        }
    }
    return true;
}

static void computeMissingNormals(const std::vector<glm::vec3>& positions,
                                  const std::vector<std::uint32_t>& indices,
                                  std::vector<glm::vec3>& normals)
{
    normals.assign(positions.size(), glm::vec3(0.0f));
    if (positions.empty()) return;

    if (indices.empty())
    {
        for (auto& n : normals) n = glm::vec3(0, 1, 0);
        return;
    }

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const auto i0 = indices[i];
        const auto i1 = indices[i + 1];
        const auto i2 = indices[i + 2];
        if (i0 >= positions.size() || i1 >= positions.size() || i2 >= positions.size())
            continue;

        glm::vec3 n = computeFaceNormal(positions[i0], positions[i1], positions[i2]);
        normals[i0] += n;
        normals[i1] += n;
        normals[i2] += n;
    }

    for (auto& n : normals)
    {
        float len = glm::length(n);
        n = (len > 1e-6f) ? (n / len) : glm::vec3(0, 1, 0);
    }
}

std::shared_ptr<Model> AssetManager::loadModelGLTF2(const std::string& relativePath, bool forceReload)
{
    const std::string abs = makeAbsPath(relativePath);

    if (!forceReload)
    {
        auto it = m_modelCache.find(abs);
        if (it != m_modelCache.end())
        {
            if (auto sp = it->second.lock())
                return sp;
        }
    }

    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf;
    std::string err, warn;

    //bool ok = loader.LoadBinaryFromFile(&gltf, &err, &warn, abs);

    const std::string ext = std::filesystem::path(abs).extension().string();
    bool ok = false;

    if (ext == ".glb" || ext == ".GLB")
    {
        ok = loader.LoadBinaryFromFile(&gltf, &err, &warn, abs);
    }
    else if (ext == ".gltf" || ext == ".GLTF")
    {
        ok = loader.LoadASCIIFromFile(&gltf, &err, &warn, abs);
    }
    else
    {
        LOG(ERROR) << "[AssetManager][glTF] Unsupported extension: " << ext;
        return nullptr;
    }

    if (!warn.empty())
        LOG(WARNING) << "[AssetManager][GLB] warn: " << warn;
    if (!ok)
    {
        LOG(ERROR) << "[AssetManager][GLB] load failed: " << abs;
        if (!err.empty()) LOG(ERROR) << err;
        return nullptr;
    }

    auto model = std::make_shared<Model>();
    model->sourcePath = abs;

    auto defaultPbrShader = loadShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
    if (!defaultPbrShader)
        LOG(ERROR) << "[AssetManager][GLB] Default PBR shader missing, materials will be invalid";

    std::vector<std::shared_ptr<Texture2D>> textureCache(gltf.textures.size());
    std::vector<std::shared_ptr<Material>> materialCache(gltf.materials.size());

    auto makeTextureParams = [&](int samplerIndex, ColorSpace cs) {
        TextureParams params;
        params.colorSpace = cs;
        params.flipY = false;

        if (samplerIndex >= 0 && samplerIndex < static_cast<int>(gltf.samplers.size()))
        {
            const auto& s = gltf.samplers[samplerIndex];
            if (s.wrapS >= 0) params.wrapS = s.wrapS;
            if (s.wrapT >= 0) params.wrapT = s.wrapT;
            if (s.minFilter >= 0)
            {
                params.minFilter = s.minFilter;
                params.generateMipmaps =
                    (s.minFilter == GL_NEAREST_MIPMAP_NEAREST ||
                     s.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
                     s.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
                     s.minFilter == GL_LINEAR_MIPMAP_LINEAR);
            }
            if (s.magFilter >= 0) params.magFilter = s.magFilter;
        }
        return params;
    };

    auto loadTextureFromIndex = [&](int texIndex, ColorSpace cs) -> std::shared_ptr<Texture2D>
    {
        if (texIndex < 0 || texIndex >= static_cast<int>(gltf.textures.size()))
            return nullptr;

        if (textureCache[texIndex]) return textureCache[texIndex];

        const auto& tex = gltf.textures[texIndex];
        const int imageIndex = tex.source;
        if (imageIndex < 0 || imageIndex >= static_cast<int>(gltf.images.size()))
            return nullptr;

        const auto& image = gltf.images[imageIndex];
        TextureParams params = makeTextureParams(tex.sampler, cs);

        std::filesystem::path pseudoPath = std::filesystem::path(abs) /
                                           ("#image_" + std::to_string(imageIndex));
        const std::string key = makeTextureKey(pseudoPath.string(), params);

        if (!forceReload)
        {
            auto it = m_textureCache.find(key);
            if (it != m_textureCache.end())
            {
                if (auto cached = it->second.lock())
                {
                    textureCache[texIndex] = cached;
                    return cached;
                }
            }
        }

        auto tex2D = std::make_shared<Texture2D>();
        bool texOk = false;
        if (!image.image.empty())
        {
            texOk = tex2D->loadFromMemory(image.image.data(),
                                          image.width,
                                          image.height,
                                          image.component,
                                          params);
        }
        else if (!image.uri.empty())
        {
            const std::filesystem::path base = std::filesystem::path(abs).parent_path();
            const std::string imgAbs = (base / image.uri).lexically_normal().string();
            texOk = tex2D->loadFromFile(imgAbs, params);
        }

        if (!texOk || !(*tex2D))
        {
            LOG(ERROR) << "[AssetManager][GLB] texture load failed, index=" << texIndex;
            return nullptr;
        }

        m_textureCache[key] = tex2D;
        textureCache[texIndex] = tex2D;
        return tex2D;
    };

    auto loadMaterialFromIndex = [&](int matIndex) -> std::shared_ptr<Material>
    {
        if (matIndex < 0 || matIndex >= static_cast<int>(gltf.materials.size()))
            return nullptr;
        if (materialCache[matIndex]) return materialCache[matIndex];

        const auto& gm = gltf.materials[matIndex];
        auto mat = std::make_shared<Material>();
        mat->shader = defaultPbrShader;

        const auto& pbr = gm.pbrMetallicRoughness;
        if (pbr.baseColorFactor.size() == 4)
        {
            mat->baseColorFactor = glm::vec4(pbr.baseColorFactor[0],
                                             pbr.baseColorFactor[1],
                                             pbr.baseColorFactor[2],
                                             pbr.baseColorFactor[3]);
        }
        mat->metallicFactor = static_cast<float>(pbr.metallicFactor);
        mat->roughnessFactor = static_cast<float>(pbr.roughnessFactor);

        mat->baseColorMap = loadTextureFromIndex(pbr.baseColorTexture.index, ColorSpace::SRGB);
        mat->metallicRoughnessMap = loadTextureFromIndex(pbr.metallicRoughnessTexture.index, ColorSpace::Linear);

        mat->normalMap = loadTextureFromIndex(gm.normalTexture.index, ColorSpace::Linear);
        mat->normalScale = static_cast<float>(gm.normalTexture.scale);

        mat->occlusionMap = loadTextureFromIndex(gm.occlusionTexture.index, ColorSpace::Linear);
        mat->occlusionStrength = static_cast<float>(gm.occlusionTexture.strength);

        mat->emissiveMap = loadTextureFromIndex(gm.emissiveTexture.index, ColorSpace::SRGB);
        if (gm.emissiveFactor.size() == 3)
        {
            mat->emissiveFactor = glm::vec3(gm.emissiveFactor[0],
                                            gm.emissiveFactor[1],
                                            gm.emissiveFactor[2]);
        }
        mat->doubleSided = gm.doubleSided;
        mat->alphaBlend = (gm.alphaMode == "BLEND");
        mat->clampParams();

        materialCache[matIndex] = mat;
        return mat;
    };

    auto processPrimitive = [&](const tinygltf::Primitive& prim,
                                const glm::mat4& nodeTransform,
                                const std::string& name) -> std::optional<Model::Submesh>
    {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES)
        {
            LOG(WARNING) << "[AssetManager][GLB] skip non-triangle primitive";
            return std::nullopt;
        }

        auto itPos = prim.attributes.find("POSITION");
        if (itPos == prim.attributes.end())
        {
            LOG(ERROR) << "[AssetManager][GLB] primitive missing POSITION";
            return std::nullopt;
        }

        std::vector<glm::vec3> positions;
        if (!readAccessorVec3(gltf, itPos->second, positions))
        {
            LOG(ERROR) << "[AssetManager][GLB] read POSITION failed";
            return std::nullopt;
        }

        std::vector<glm::vec3> normals;
        auto itNormal = prim.attributes.find("NORMAL");
        bool hasNormals = itNormal != prim.attributes.end() &&
                          readAccessorVec3(gltf, itNormal->second, normals);

        std::vector<glm::vec2> uvs;
        auto itUV = prim.attributes.find("TEXCOORD_0");
        bool hasUV = itUV != prim.attributes.end() &&
                     readAccessorVec2(gltf, itUV->second, uvs);

        std::vector<std::uint32_t> indices;
        bool hasIndex = prim.indices >= 0 && readIndices(gltf, prim.indices, indices);
        if (!hasIndex)
        {
            indices.resize(positions.size());
            std::iota(indices.begin(), indices.end(), 0);
        }

        if (!hasNormals)
            computeMissingNormals(positions, indices, normals);

        std::vector<Mesh::Vertex> verts;
        verts.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i)
        {
            Mesh::Vertex v;
            v.position = positions[i];
            v.normal   = (i < normals.size()) ? normals[i] : glm::vec3(0, 1, 0);
            v.uv       = (hasUV && i < uvs.size()) ? uvs[i] : glm::vec2(0.0f);
            verts.push_back(v);
        }

        auto mesh = std::make_shared<Mesh>(verts, indices);
        Model::Submesh sm;
        sm.name = name;
        sm.mesh = mesh;
        sm.local = nodeTransform;
        sm.material = loadMaterialFromIndex(prim.material);
        return sm;
    };

    auto processNode = [&](auto&& self, int nodeIndex, const glm::mat4& parentMat) -> void
    {
        if (nodeIndex < 0 || nodeIndex >= static_cast<int>(gltf.nodes.size()))
            return;
        const auto& node = gltf.nodes[nodeIndex];

        glm::mat4 nodeMat = parentMat * composeNodeTransform(node);

        if (node.mesh >= 0 && node.mesh < static_cast<int>(gltf.meshes.size()))
        {
            const auto& mesh = gltf.meshes[node.mesh];
            for (size_t p = 0; p < mesh.primitives.size(); ++p)
            {
                auto sm = processPrimitive(mesh.primitives[p], nodeMat,
                                           mesh.name.empty() ? ("primitive_" + std::to_string(p)) : mesh.name);
                if (sm.has_value())
                    model->submeshes.push_back(std::move(*sm));
            }
        }

        for (int child : node.children)
            self(self, child, nodeMat);
    };

    int sceneIndex = (gltf.defaultScene >= 0 && gltf.defaultScene < static_cast<int>(gltf.scenes.size()))
                     ? gltf.defaultScene
                     : (!gltf.scenes.empty() ? 0 : -1);

    if (sceneIndex < 0)
    {
        LOG(ERROR) << "[AssetManager][GLB] No scene found in glTF";
        return nullptr;
    }

    const auto& scene = gltf.scenes[sceneIndex];
    for (int node : scene.nodes)
        processNode(processNode, node, glm::mat4(1.0f));

    m_modelCache[abs] = model;
    return model;
}


std::shared_ptr<Shader> AssetManager::loadShader(const std::string& vertRelPath,
                                                 const std::string& fragRelPath,
                                                 const std::vector<std::string>& defines,
                                                 bool forceReload)
{
    const std::string vAbs = makeAbsPath(vertRelPath);
    const std::string fAbs = makeAbsPath(fragRelPath);
    const std::string key  = makeShaderKey(vAbs, fAbs, defines);

    if (!forceReload)
    {
        auto it = m_shaderCache.find(key);
        if (it != m_shaderCache.end())
        {
            if (auto sp = it->second.lock())
                return sp;
        }
    }

    auto shader = std::make_shared<Shader>();
    if (!shader->loadFromFiles(vAbs, fAbs))
        return nullptr;

    m_shaderCache[key] = shader;
    return shader;
}


void AssetManager::purgeUnused()
{
    for (auto it = m_textureCache.begin(); it != m_textureCache.end(); )
        it = it->second.expired() ? m_textureCache.erase(it) : std::next(it);

    for (auto it = m_modelCache.begin(); it != m_modelCache.end(); )
        it = it->second.expired() ? m_modelCache.erase(it) : std::next(it);

    for (auto it = m_shaderCache.begin(); it != m_shaderCache.end(); )
        it = it->second.expired() ? m_shaderCache.erase(it) : std::next(it);

}

std::string AssetManager::makeShaderKey(const std::string& vAbs,
                                        const std::string& fAbs,
                                        const std::vector<std::string>& defines)
{
    std::string key = vAbs + "|" + fAbs;
    for (auto& d : defines) key += "|D:" + d;
    return key;
}
