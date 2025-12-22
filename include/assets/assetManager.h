//
// Created by Remilia on 25-12-19.
//

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

#include <graphics/texture.h>
#include <assets/model.h>
#include <graphics/shader.h>

struct TextureParams;

class AssetManager
{
public:
    explicit AssetManager(std::string assetRootDir);

    void setRootDir(const std::string& root);
    const std::string& rootDir() const;

    // ---------- Textures ----------
    std::shared_ptr<Texture2D> loadTexture2D(const std::string& relativePath,
                                             const TextureParams& params,
                                             bool forceReload = false);

    // ---------- Models ----------
    std::shared_ptr<Model> loadModelOBJ(const std::string& relativePath,
                                        bool forceReload = false);

    std::shared_ptr<Model> loadModelGLTF2(const std::string& relativePath,
                                        bool forceReload = false);

    // ---------- Shaders ---------
    std::shared_ptr<Shader> loadShader(const std::string& vertRelPath,
                                   const std::string& fragRelPath,
                                   const std::vector<std::string>& defines = {},
                                   bool forceReload = false);

    void purgeUnused();

private:
    std::string makeAbsPath(const std::string& relativePath) const;
    static std::string makeTextureKey(const std::string& absPath, const TextureParams& params);

private:
    std::string m_root;

    static std::string makeShaderKey(const std::string& vAbs,
                                 const std::string& fAbs,
                                 const std::vector<std::string>& defines);

    std::unordered_map<std::string, std::weak_ptr<Shader>> m_shaderCache;

    std::unordered_map<std::string, std::weak_ptr<Texture2D>> m_textureCache;
    std::unordered_map<std::string, std::weak_ptr<Model>> m_modelCache;
};

#endif //ASSETMANAGER_H
