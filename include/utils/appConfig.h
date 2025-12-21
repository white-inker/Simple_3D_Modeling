//
// Created by Remilia on 25-12-19.
//

#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include <glm/glm.hpp>

struct FrameConfig
{
    glm::vec4 clearColor{1, 1, 1, 1};

    bool clearColorBuffer = true;
    bool clearDepthBuffer = true;

    bool enableDepthTest   = true;
    bool enableCullFace    = true;
    bool enableBlend       = false;
    bool enableMultisample = true;
};

struct WindowConfig
{
    std::string windowTitle;
    int windowWidth  = 800;
    int windowHeight = 600;

    bool windowResizable = false;
    bool vSync = false;

    bool msaa = false;
    int msaaSamples = 4;

    FrameConfig frame{};
};

struct DirectionalLightConfig
{
    bool enabled = true;
    glm::vec3 direction{-0.2f, -1.0f, -0.3f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

struct PointLightConfig
{
    bool enabled = true;
    glm::vec3 position{0,0,0};
    glm::vec3 color{1,1,1};
    float intensity = 1.0f;
    float range = 10.0f;
};

struct LightingConfig
{
    glm::vec3 ambient{0.03f, 0.03f, 0.03f};
    DirectionalLightConfig directional{};
    std::vector<PointLightConfig> points{};
};

class AppConfig
{
public:
    std::string assetRootDir;

    std::unordered_map<std::string, WindowConfig> windows;

    std::pair<int, int> glVersion{3, 3};

    LightingConfig defaultLighting{};

    static AppConfig& instance();

private:
    AppConfig() = default;
    void init(const std::string& configPath);

    static bool isInitialized;
};

#endif //APPCONFIG_H
