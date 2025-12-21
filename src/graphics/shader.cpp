//
// Created by Remilia on 25-12-18.
//

#include <graphics/shader.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader()
{
    destroy();
}

Shader::Shader(Shader&& other) noexcept
{
    *this = std::move(other);
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other) return *this;

    destroy();

    program = other.program;
    other.program = 0;

    uniformCache = std::move(other.uniformCache);
    return *this;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
{
    const std::string vs = readTextFile(vertexPath);
    const std::string fs = readTextFile(fragmentPath);
    if (vs.empty() || fs.empty())
        return false;

    return loadFromSource(vs, fs);
}

bool Shader::loadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    destroy();
    uniformCache.clear();

    std::string err;
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSrc, err);
    if (!vs)
    {
        LOG(ERROR) << "Vertex shader compile error: " << err;
        return false;
    }

    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSrc, err);
    if (!fs)
    {
        LOG(ERROR) << "Fragment shader compile error: " << err;
        glDeleteShader(vs);
        return false;
    }

    program = link(vs, fs, err);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!program)
    {
        LOG(ERROR) << "Program link error: " << err;
        return false;
    }

    return true;
}

void Shader::bind() const
{
    glUseProgram(program);
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::setInt(const std::string& name, int v)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniform1i(loc, v);
}

void Shader::setFloat(const std::string& name, float v)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniform1f(loc, v);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(v));
}

void Shader::setVec4(const std::string& name, const glm::vec4& v)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniform4fv(loc, 1, glm::value_ptr(v));
}

void Shader::setMat3(const std::string& name, const glm::mat3& m)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setMat4(const std::string& name, const glm::mat4& m)
{
    GLint loc = getLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

GLint Shader::getLocation(const std::string& name)
{
    auto it = uniformCache.find(name);
    if (it != uniformCache.end())
        return it->second;

    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc >= 0) uniformCache.emplace(name, loc);
    else LOG(ERROR) << "No such Uniform" ;
    return loc;
}

std::string Shader::readTextFile(const std::string& path)
{
    std::ifstream ifs(path, std::ios::in);
    if (!ifs.is_open())
    {
        LOG(ERROR) << path << " not found\n";
        return {};
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& src, std::string& outError)
{
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        outError = log;
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint Shader::link(GLuint vs, GLuint fs, std::string& outError)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        outError = log;
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

void Shader::destroy()
{
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
}
