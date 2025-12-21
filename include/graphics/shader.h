//
// Created by Remilia on 25-12-18.
//

#ifndef SHADER_H
#define SHADER_H

#include <utils/common.h>
#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // 从文件
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // 从源码
    bool loadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);

    void bind() const;
    static void unbind();

    GLuint id() const { return program; }
    explicit operator bool() const { return program != 0; }

    // uniform setters
    void setInt(const std::string& name, int v);
    void setFloat(const std::string& name, float v);
    void setVec3(const std::string& name, const glm::vec3& v);
    void setVec4(const std::string& name, const glm::vec4& v);
    void setMat3(const std::string& name, const glm::mat3& m);
    void setMat4(const std::string& name, const glm::mat4& m);

private:
    static std::string readTextFile(const std::string& path);

    static GLuint compile(GLenum type, const std::string& src, std::string& outError);
    static GLuint link(GLuint vs, GLuint fs, std::string& outError);

    GLint getLocation(const std::string& name);

    void destroy();

private:
    GLuint program = 0;
    std::unordered_map<std::string, GLint> uniformCache;
};


#endif //SHADER_H
