//
// Created by Omari on 25-12-17.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <filesystem>
#include <utils/common.h>
#include <utils/appConfig.h>
#include <unordered_map>
#include <assets/assetManager.h>

class Scene;
class Renderer;
class Window;

class Application
{
public:
    explicit Application(std::string name);
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    virtual ~Application();

    void run();
    void init();

protected:
    virtual void onUpdate(float dt) {}
    virtual void onRenderWindow(const std::string& name, Window& window, float dt) {}
    virtual void render();
    //virtual void initAssets() {};
    virtual void initWindowCamera(const WindowConfig& config, Window& window);
    virtual void initLightFromConfig();
    virtual void initSceneEntity() {};
    virtual void initWindowInput(const WindowConfig& config, Window& win);

private:
    void createWindows();
    static void initWindowFPSCamera(const WindowConfig& config, Window& window);
    static void initWindowOrbitCamera(const WindowConfig& config, Window& window);
    [[nodiscard]] bool isRunning() const;
    void handleScreenshotRequest(const std::string& windowName, Window& win);
    [[nodiscard]] std::filesystem::path buildScreenshotPath(const std::string& windowName) const;

    void updateTime();

    static void errorCallback(int error, const char* description);

protected:
    [[nodiscard]] std::filesystem::path getExecutableDir() const;

    std::unordered_map<std::string, std::unique_ptr<Window>> windows;

    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Renderer> renderer;
    std::filesystem::path screenshotDir;

    // timing
    float timeNow = 0.0f;
    float timeLast = 0.0f;
    float deltaTime = 0.0f;
    std::string exeName;
};

#endif //APPLICATION_H
