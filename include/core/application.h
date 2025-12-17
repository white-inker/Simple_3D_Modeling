//
// Created by Omari on 25-12-17.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <core/window.h>
#include <utils/common.h>
#include <interaction/inputContext.h>
#include <glm/glm.hpp>
#include <unordered_map>

struct WindowConfig
{
    std::string windowTitle;
    int windowWidth = 800;
    int windowHeight = 600;
    bool windowResizable = false;
    bool vSync = false;
    bool msaa = false;
};

class AppConfig
{
public:
    std::string assetRootDir;
    std::unordered_map<std::string, WindowConfig> windows;
    std::pair<int, int> glVersion;
    glm::vec4 backgroundColor{1.0f, 1.0f, 1.0f, 1.0f};
    static AppConfig& instance();
private:
    AppConfig() = default;
    void init(const std::string& configPath);
    static bool isInitialized;
};

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    virtual ~Application();
    void run();
private:
    void init();
    void createWindows();
    bool isRunning();
    void updateTime();
    virtual void renderFrame();
    static void errorCallback(int error, const char* description);

    std::unordered_map<std::string, std::unique_ptr<Window>> windows;
};

#endif //APPLICATION_H
