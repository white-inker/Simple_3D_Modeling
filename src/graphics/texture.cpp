//
// Created by Remilia on 25-12-19.
//
#include <graphics/texture.h>

#include <algorithm>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <third_party/stb_image.h>

Texture2D::~Texture2D()
{
    destroy();
}

Texture2D::Texture2D(Texture2D&& other) noexcept
{
    *this = std::move(other);
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    if (this == &other) return *this;
    destroy();

    m_id = other.m_id; other.m_id = 0;
    m_width = other.m_width; other.m_width = 0;
    m_height = other.m_height; other.m_height = 0;
    m_channels = other.m_channels; other.m_channels = 0;
    m_path = std::move(other.m_path);

    return *this;
}

void Texture2D::destroy()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_width = m_height = m_channels = 0;
    m_path.clear();
}

void Texture2D::bind(GLuint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::chooseFormat(int channels, ColorSpace cs, GLint& internalFmt, GLenum& dataFmt)
{
    // dataFmt：上传数据的格式；internalFmt：GPU 内部存储格式
    if (channels == 4)
    {
        dataFmt = GL_RGBA;
        internalFmt = (cs == ColorSpace::SRGB) ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    }
    else if (channels == 3)
    {
        dataFmt = GL_RGB;
        internalFmt = (cs == ColorSpace::SRGB) ? GL_SRGB8 : GL_RGB8;
    }
    else // channels == 1
    {
        dataFmt = GL_RED;
        internalFmt = GL_R8;
    }
}

bool Texture2D::loadFromFile(const std::string& path, const TextureParams& params)
{
    destroy();
    m_path = path;

    stbi_set_flip_vertically_on_load(params.flipY ? 1 : 0);

    int w=0,h=0,chs=0; // channel
    int req = params.forceChannels.has_value() ? *params.forceChannels : 0;

    unsigned char* data = stbi_load(path.c_str(), &w, &h, &chs, req);
    if (!data)
    {
        LOG(ERROR) << "[Texture2D] Failed to load: " << path;
        return false;
    }

    if (req != 0) chs = req;

    m_width = w;
    m_height = h;
    m_channels = chs;

    GLint internalFmt = GL_RGBA8;
    GLenum dataFmt = GL_RGBA;
    chooseFormat(chs, params.colorSpace, internalFmt, dataFmt);

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0, dataFmt, GL_UNSIGNED_BYTE, data);

    // restore (可选)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // wrap/filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magFilter);

    if (params.generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    // Anisotropic filtering (optional)
    if (params.anisotropy > 0.0f && supportsAnisotropy())
    {
        float maxA = maxAnisotropy();
        float a = std::clamp(params.anisotropy, 1.0f, maxA);
        // 需要扩展：GL_EXT_texture_filter_anisotropic
        glTexParameterf(GL_TEXTURE_2D, 0x84FE /*GL_TEXTURE_MAX_ANISOTROPY_EXT*/, a);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return true;
}

bool Texture2D::supportsAnisotropy()
{
    const char* ext = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (!ext) return false;
    return std::string(ext).find("GL_EXT_texture_filter_anisotropic") != std::string::npos;
}

float Texture2D::maxAnisotropy()
{
    float v = 1.0f;
    glGetFloatv(0x84FF /*GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT*/, &v);
    return std::max(1.0f, v);
}
