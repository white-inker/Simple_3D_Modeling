//
// Created by Omari on 25-12-17.
//

#include <utils/glogWrapper.h>
#include <filesystem>

bool GlogWrapper::s_initialized = false;

GlogWrapper::GlogWrapper(const std::string& programName,
                     bool logToConsole ,
                     int minLogLevel)
{
    Init(programName, logToConsole, minLogLevel);
}

GlogWrapper::~GlogWrapper()
{
    Shutdown();
}

void GlogWrapper::Init(const std::string& programName,
                       bool logToConsole,
                       int minLogLevel)
{
    if (s_initialized)
    {
        return;
    }

    std::filesystem::path exeDir =
    std::filesystem::absolute(programName).parent_path();

    std::filesystem::path logDir = exeDir / "logs";

    // 创建日志目录（如果不存在）
    try
    {
        std::filesystem::create_directories(logDir);
    }
    catch (...) { }

    // 日志输出目录
    FLAGS_log_dir = "./logs";
    google::InitGoogleLogging(programName.c_str());

    // 同时输出到控制台
    FLAGS_alsologtostderr = logToConsole;

    // 彩色输出
    FLAGS_colorlogtostderr = true;

    // 设置最小日志级别
    FLAGS_minloglevel = minLogLevel;

    // 每个日志文件最大大小（MB）
    FLAGS_max_log_size = 10;

    google::InstallFailureSignalHandler();

    s_initialized = true;
}

void GlogWrapper::Shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    google::ShutdownGoogleLogging();
    s_initialized = false;
}

bool GlogWrapper::IsInitialized()
{
    return s_initialized;
}
