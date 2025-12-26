#include <core/application.h>

#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include <glad/glad.h>

#include <utils/appConfig.h>
#include <core/window.h>
#include <graphics/renderer.h>
#include <scene/scene.h>

#include <utils/glogWrapper.h>
#include <scene/directionalLight.h>
#include <scene/pointLight.h>

Application::Application(std::string name) : exeName(std::move(name))
{
}

Application::~Application()
{
    renderer.reset();
    scene.reset();
    windows.clear();
    glfwTerminate();
}

void Application::init()
{
    glfwSetErrorCallback(errorCallback);

    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("glfwInit failed");

    // OpenGL 全局 hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, AppConfig::instance().glVersion.first);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, AppConfig::instance().glVersion.second);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    createWindows();

    screenshotDir = getExecutableDir() / "screenshots";
    std::error_code ec;
    std::filesystem::create_directories(screenshotDir, ec);

    auto assetPath = getExecutableDir() / AppConfig::instance().assetRootDir;

    renderer = std::make_unique<Renderer>();
    scene = std::make_unique<Scene>();
    assetManager = std::make_unique<AssetManager>(assetPath.string());
    //initAssets();
    initLightFromConfig();
    initSceneEntity();

    // 初始时间
    timeLast = static_cast<float>(glfwGetTime());
}

void Application::createWindows()
{
    // 为了让多窗口资源可共享：后续窗口 share 第一个窗口的 context
    GLFWwindow* sharedContext = nullptr;

    std::vector<std::string> names;
    names.reserve(AppConfig::instance().windows.size());
    for (auto& [name, _] : AppConfig::instance().windows) names.push_back(name);

    auto itMain = std::find(names.begin(), names.end(), "mainWindow");
    if (itMain != names.end())
        std::rotate(names.begin(), itMain, itMain + 1);
    else
        std::sort(names.begin(), names.end());

    bool gladLoaded = false;

    for (const auto& name : names)
    {
        const auto& wc = AppConfig::instance().windows.at(name);

        // 每个窗口的 hint 需要在创建前设置
        glfwWindowHint(GLFW_RESIZABLE, wc.windowResizable ? GLFW_TRUE : GLFW_FALSE);

        if (wc.msaa)
            glfwWindowHint(GLFW_SAMPLES, wc.msaaSamples);
        else
            glfwWindowHint(GLFW_SAMPLES, 0);

        GLFWwindow* w = glfwCreateWindow(
            wc.windowWidth,
            wc.windowHeight,
            wc.windowTitle.c_str(),
            nullptr,
            sharedContext
        );

        if (!w)
            throw std::runtime_error("glfwCreateWindow failed: " + name);

        // 插入 Window 对象
        auto [iter, ok] = windows.emplace(name, std::make_unique<Window>(w));
        initWindowCamera(wc, *iter->second);
        initWindowInput(wc, *iter->second);

        glfwMakeContextCurrent(w);

        // GLAD 只加载一次
        if (!gladLoaded)
        {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
                throw std::runtime_error("gladLoadGLLoader failed");

            gladLoaded = true;

            LOG(INFO) << "OpenGL";
            LOG(INFO) << "+ version:    " << glGetString(GL_VERSION);
            LOG(INFO) << "+ renderer:   " << glGetString(GL_RENDERER);
            LOG(INFO) << "+ glsl:       " << glGetString(GL_SHADING_LANGUAGE_VERSION);

            // 第一个窗口创建完后作为 share 源
            sharedContext = w;
        }

        // 初始化窗口子系统
        iter->second->init();

        glfwSwapInterval(wc.vSync ? 1 : 0);
    }
}

void Application::initLightFromConfig()
{
    // Scene 管灯光：从 AppConfig.defaultLighting 初始化
    auto& Lcfg = AppConfig::instance().defaultLighting;
    auto& L = scene->getLighting();
    L.ambient = Lcfg.ambient;

    // directional
    if (Lcfg.directional.enabled)
    {
        auto dir = std::make_unique<DirectionalLight>();
        dir->direction = Lcfg.directional.direction;
        dir->color = Lcfg.directional.color;
        dir->intensity = Lcfg.directional.intensity;
        L.setDirectional(std::move(dir));
    }

    // points
    for (const auto& pc : Lcfg.points)
    {
        if (!pc.enabled) continue;
        auto& p = L.addPointLight();
        p.position  = pc.position;
        p.color     = pc.color;
        p.intensity = pc.intensity;
        p.range     = pc.range;
    }
}

void Application::initWindowCamera(const WindowConfig &config, Window &window)
{
    // 默认 FPS 相机
    initWindowFPSCamera(config, window);
    // 你可以在下面设置相机参数
}


void Application::run()
{
    while (isRunning())
    {
        glfwPollEvents();

        updateTime();
        onUpdate(deltaTime);

        render();
    }
}

void Application::render()
{
    for (auto& [name, win] : windows)
    {
        GLFWwindow* w = win->window;
        glfwMakeContextCurrent(w);

        win->handleInput(deltaTime);

        // per-window FrameConfig -> Renderer::FrameParams
        const auto& wc = AppConfig::instance().windows.at(name);

        Renderer::FrameParams fp;
        fp.clearColor         = wc.frame.clearColor;
        fp.clearColorBuffer   = wc.frame.clearColorBuffer;
        fp.clearDepthBuffer   = wc.frame.clearDepthBuffer;
        fp.enableDepthTest    = wc.frame.enableDepthTest;
        fp.enableCullFace     = wc.frame.enableCullFace;
        fp.enableBlend        = wc.frame.enableBlend;
        fp.enableMultisample  = (wc.msaa && wc.frame.enableMultisample);

        renderer->beginFrame(w, fp);

        const auto view = win->cameraController->getCamera().getViewMatrix();
        const auto proj = win->cameraController->getCamera().getProjectionMatrix();
        const auto viewPos = win->cameraController->getCamera().getPosition();

        renderer->renderScene(*scene, view, proj, viewPos);

        renderer->endFrame();

        onRenderWindow(name, *win, deltaTime);

        handleScreenshotRequest(name, *win);

        glfwSwapBuffers(w);
    }
}

bool Application::isRunning() const
{
    for (auto& [_, win] : windows)
        if (!glfwWindowShouldClose(win->window))
            return true;
    return false;
}

void Application::updateTime()
{
    timeNow = static_cast<float>(glfwGetTime());
    deltaTime = timeNow - timeLast;
    timeLast = timeNow;
}

void Application::errorCallback(int error, const char* description)
{
    LOG(ERROR) << "GLFW Error(" << error << "): " << description;
}

std::filesystem::path Application::getExecutableDir() const
{
    return std::filesystem::absolute(exeName).parent_path();
}

void Application::initWindowFPSCamera(const WindowConfig &config, Window &window)
{
    auto camera = FpsCamera();
    window.initCameraController(camera);
}

void Application::initWindowOrbitCamera(const WindowConfig &config, Window &window)
{
    auto camera = OrbitCamera();
    window.initCameraController(camera);
}

void Application::initWindowInput(const WindowConfig& config, Window& win)
{
    glfwSetInputMode(win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    win.input.OnMouseMove(static_cast<float>(config.windowWidth) * 0.5f, static_cast<float>(config.windowHeight) * 0.5f);
    win.input.Update();
    const auto pos = win.input.GetMousePosition();
    glfwSetCursorPos(win.window, pos.x , pos.y);
}

void Application::handleScreenshotRequest(const std::string& windowName, Window& win)
{
    if (!renderer) return;
    if (!win.consumeScreenshotRequest()) return;

    const auto path = buildScreenshotPath(windowName);
    if (path.empty()) return;

    const auto parent = path.parent_path();
    std::error_code ec;
    if (!parent.empty())
        std::filesystem::create_directories(parent, ec);

    renderer->saveScreenshot(path);
}

std::filesystem::path Application::buildScreenshotPath(const std::string& windowName) const
{
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto tt = clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << windowName << "_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << "_" << std::setw(3) << std::setfill('0') << millis.count()
        << ".png";

    const auto dir = screenshotDir.empty() ? getExecutableDir() / "screenshots" : screenshotDir;
    return dir / oss.str();
}





