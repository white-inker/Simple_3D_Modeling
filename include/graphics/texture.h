//
// Created by Remilia on 25-12-19.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <utils/common.h>
#include <string>
#include <optional>

enum class ColorSpace
{
    Linear,
    SRGB
};

struct TextureParams
{
    ColorSpace colorSpace = ColorSpace::Linear;

    bool flipY = true;
    bool generateMipmaps = true;

    GLint wrapS = GL_REPEAT;
    GLint wrapT = GL_REPEAT;

    // 如果 generateMipmaps=true，建议 minFilter 用 *_MIPMAP_*
    GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLint magFilter = GL_LINEAR;

    // 0 表示不设置；否则会 clamp 到硬件支持范围
    float anisotropy = 8.0f;

    // 有些贴图需要强制 4 通道（例如保证 RGBA）
    // std::nullopt 表示保持原始通道数；可设为 3 或 4
    std::optional<int> forceChannels = std::nullopt;
};

class Texture2D
{
public:
    Texture2D() = default;
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    bool loadFromFile(const std::string& path, const TextureParams& params = {});
    bool loadFromMemory(const unsigned char* data,
                        int width,
                        int height,
                        int channels,
                        const TextureParams& params = {});
    void destroy();

    void bind(GLuint unit = 0) const;

    GLuint id() const { return m_id; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    int channels() const { return m_channels; }
    const std::string& path() const { return m_path; }

    explicit operator bool() const { return m_id != 0; }

private:
    static bool supportsAnisotropy();
    static float maxAnisotropy();
    static void Texture2D::chooseFormat(int channels, ColorSpace cs, GLint& internalFmt, GLenum& dataFmt);

private:
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    std::string m_path;
};


#endif //TEXTURE_H
