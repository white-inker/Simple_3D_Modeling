
#include <assets/assetManager.h>

#include <filesystem>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <graphics/mesh.h>

// tinyobjloader
#include <third_party/tiny_obj_loader.h>

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

    // 每个 shape 可能包含多个 material id，我们按 material 分 submesh（更合理）
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

                std::uint32_t newId = static_cast<std::uint32_t>(b.verts.size());
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
            //sm.material = nullptr;           // 先不从 mtl 创建 PBR 材质（你后面要我也可以加）
            sm.local = glm::mat4(1.0f);

            model->submeshes.push_back(std::move(sm));
            ++smIndex;
        }
    }

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
