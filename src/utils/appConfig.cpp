//
// Created by Remilia on 25-12-19.
//

#include <utils/appConfig.h>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <utils/common.h>

bool AppConfig::isInitialized = false;

AppConfig& AppConfig::instance()
{
    static AppConfig inst;
    if (!isInitialized)
    {
        inst.init(appConfigPath);
        isInitialized = true;
    }
    return inst;
}

static glm::vec4 readVec4(const nlohmann::json& j, const glm::vec4& def)
{
    if (!j.is_array() || j.size() < 4) return def;
    return {j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>()};
}

static glm::vec3 readVec3(const nlohmann::json& j, const glm::vec3& def)
{
    if (!j.is_array() || j.size() < 3) return def;
    return {j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>()};
}

void AppConfig::init(const std::string& configPath)
{
    using json = nlohmann::json;

    std::ifstream f(configPath);
    if (!f.is_open())
        throw std::runtime_error("App config file not found: " + configPath);

    json cfg;
    f >> cfg;

    assetRootDir = cfg.value("assetRootDir", "");

    // glVersion
    if (cfg.contains("glVersion"))
    {
        glVersion.first  = cfg["glVersion"].value("major", 3);
        glVersion.second = cfg["glVersion"].value("minor", 3);
    }

    // windows
    windows.clear();

    if (cfg.contains("windows"))
    {
        for (auto& [name, jw] : cfg["windows"].items())
        {
            WindowConfig wc;
            wc.windowTitle     = jw.value("windowTitle", name);
            wc.windowWidth     = jw.value("windowWidth", 800);
            wc.windowHeight    = jw.value("windowHeight", 600);
            wc.windowResizable = jw.value("windowResizable", false);
            wc.vSync           = jw.value("vSync", false);

            wc.msaa        = jw.value("msaa", false);
            wc.msaaSamples = jw.value("msaaSamples", 4);

            if (jw.contains("frame"))
            {
                auto jf = jw["frame"];
                wc.frame.clearColor         = jf.contains("clearColor") ? readVec4(jf["clearColor"], wc.frame.clearColor) : wc.frame.clearColor;
                wc.frame.clearColorBuffer   = jf.value("clearColorBuffer", true);
                wc.frame.clearDepthBuffer   = jf.value("clearDepthBuffer", true);
                wc.frame.enableDepthTest    = jf.value("enableDepthTest", true);
                wc.frame.enableCullFace     = jf.value("enableCullFace", true);
                wc.frame.enableBlend        = jf.value("enableBlend", false);
                wc.frame.enableMultisample  = jf.value("enableMultisample", true);
            }

            windows[name] = wc;
        }
    }
    else
    {
        WindowConfig wc;
        auto jw = cfg["mainWindow"];
        wc.windowTitle     = jw.value("windowTitle", "mainWindow");
        wc.windowWidth     = jw.value("windowWidth", 800);
        wc.windowHeight    = jw.value("windowHeight", 600);
        wc.windowResizable = jw.value("windowResizable", false);
        wc.vSync           = jw.value("vSync", false);
        wc.msaa            = jw.value("msaa", false);
        wc.msaaSamples     = jw.value("msaaSamples", 4);

        // 旧 backgroundColor 作为默认 clearColor
        if (cfg.contains("backgroundColor"))
            wc.frame.clearColor = readVec4(cfg["backgroundColor"], wc.frame.clearColor);

        windows["mainWindow"] = wc;
    }

    // lighting（Scene 的默认灯光）
    if (cfg.contains("lighting"))
    {
        auto jl = cfg["lighting"];
        defaultLighting.ambient = jl.contains("ambient") ? readVec3(jl["ambient"], defaultLighting.ambient) : defaultLighting.ambient;

        if (jl.contains("directional"))
        {
            auto jd = jl["directional"];
            defaultLighting.directional.enabled   = jd.value("enabled", true);
            defaultLighting.directional.direction = jd.contains("direction") ? readVec3(jd["direction"], defaultLighting.directional.direction) : defaultLighting.directional.direction;
            defaultLighting.directional.color     = jd.contains("color") ? readVec3(jd["color"], defaultLighting.directional.color) : defaultLighting.directional.color;
            defaultLighting.directional.intensity = jd.value("intensity", 1.0f);
        }

        defaultLighting.points.clear();
        if (jl.contains("points") && jl["points"].is_array())
        {
            for (auto& jp : jl["points"])
            {
                PointLightConfig pc;
                pc.enabled   = jp.value("enabled", true);
                pc.position  = jp.contains("position") ? readVec3(jp["position"], pc.position) : pc.position;
                pc.color     = jp.contains("color") ? readVec3(jp["color"], pc.color) : pc.color;
                pc.intensity = jp.value("intensity", 1.0f);
                pc.range     = jp.value("range", 10.0f);
                defaultLighting.points.push_back(pc);
            }
        }
    }
}
