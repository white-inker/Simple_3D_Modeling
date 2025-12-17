//
// Created by Omari on 25-12-17.
//

#include <nlohmann/json.hpp>
#include <fstream>
#include <core/application.h>
#include <utils/common.h>
#include <utils/glogWrapper.h>

bool AppConfig::isInitialized = false;

AppConfig &AppConfig::instance()
{
    static AppConfig instance;
    if (!isInitialized)
    {
        instance.init(appConfigPath);
        isInitialized = true;
    }
    return instance;
}

void AppConfig::init(const std::string& configPath)
{
    using json = nlohmann::json;
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) throw std::runtime_error("Server config file not found");
    json config;
    configFile >> config;
    assetRootDir = config["assetRootDir"].get<std::string>();
    // windows
    windows["mainWindow"].windowTitle = config["mainWindow"]["windowTitle"].get<std::string>();
    windows["mainWindow"].windowWidth = config["mainWindow"]["windowWidth"].get<int>();
    windows["mainWindow"].windowHeight = config["mainWindow"]["windowHeight"].get<int>();
    windows["mainWindow"].windowResizable = config["mainWindow"]["windowResizable"].get<bool>();
    windows["mainWindow"].vSync = config["mainWindow"]["vSync"].get<bool>();
    windows["mainWindow"].msaa = config["mainWindow"]["msaa"].get<bool>();

    glVersion.first = config["glVersion"]["major"].get<int>();
    glVersion.second = config["glVersion"]["minor"].get<int>();

    backgroundColor.x = config["backgroundColor"].at(0).get<float>();
    backgroundColor.y = config["backgroundColor"].at(1).get<float>();
    backgroundColor.z = config["backgroundColor"].at(2).get<float>();
    backgroundColor.w = config["backgroundColor"].at(3).get<float>();
}

Application::Application()
{
    init();
}

Application::~Application()
{
    glfwTerminate();
}

void Application::init()
{
    glfwSetErrorCallback(errorCallback);

    // init glfw
    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("init glfw failure");
    }

    // create glfw window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, AppConfig::instance().glVersion.first);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, AppConfig::instance().glVersion.second);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    createWindows();

    LOG(INFO) << "OpenGL";
    LOG(INFO) << "+ version:    " << glGetString(GL_VERSION);
    LOG(INFO) << "+ renderer:   " << glGetString(GL_RENDERER);
    LOG(INFO) << "+ glsl:       " << glGetString(GL_SHADING_LANGUAGE_VERSION);
}

void Application::createWindows()
{
    bool isGladLoaded = false;
    for (auto &[windowName, windowConfig] : AppConfig::instance().windows)
    {
        int w, h;
        glfwWindowHint(GLFW_RESIZABLE, windowConfig.windowResizable);
        if (windowConfig.msaa)
        {
            glfwWindowHint(GLFW_SAMPLES, 4);
        }

        auto window = glfwCreateWindow(windowConfig.windowWidth, windowConfig.windowHeight,
            windowConfig.windowTitle.c_str(), nullptr, nullptr);

        if (window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("create glfw window failure");
        }

        auto& [iter , _] = windows.emplace(windowName, std::make_unique<Window>(window));
        glfwMakeContextCurrent(window);
        if (!isGladLoaded)
        {
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
            {
                throw std::runtime_error("glad initialization OpenGL failure");
            }
        }
        iter->second->init();

        if (windowConfig.vSync)
            glfwSwapInterval(1);
        else glfwSwapInterval(0);

        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        if (windowConfig.msaa)
            glEnable(GL_MULTISAMPLE);
    }
}

void Application::run()
{
    while (isRunning())
    {
        glfwPollEvents();
        updateTime();

        for (auto &[_, window] : windows)
        {
            glfwMakeContextCurrent(window->window);
            window->handleInput();
            window->renderFrame();
            glfwSwapBuffers(window->window);
        }
    }
}

bool Application::isRunning()
{
    for (auto& [_, window] : windows)
        if (!glfwWindowShouldClose(window->window))
            return true;
    return false;
}


void Application::renderFrame()
{
    LOG(WARNING) << "Application::renderFrame hasn't been implemented yet";
}

void Application::errorCallback(int error, const char *description)
{
    LOG(ERROR) << description;
}

void Application::updateTime()
{
    LOG(WARNING) << "Application::updateTime hasn't been implemented yet";
}





