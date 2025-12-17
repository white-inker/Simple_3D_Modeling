//
// Created by Omari on 25-12-17.
//

#include <utils/glogWrapper.h>
#include <filesystem>

bool GlogWrapper::s_initialized = false;

void GlogWrapper::Init(const std::string &programName,
                       const std::string &logDir,
                       bool logToConsole,
                       int minLogLevel)
{
    if (s_initialized)
    {
        return;
    }

    // 创建日志目录（如果不存在）
    try
    {
        std::filesystem::create_directories(logDir);
    }
    catch (...) { }

    google::InitGoogleLogging(programName.c_str());

    // 日志输出目录
    FLAGS_log_dir = logDir;

    // 同时输出到控制台
    FLAGS_alsologtostderr = logToConsole;

    // 彩色输出
    FLAGS_colorlogtostderr = true;

    // 设置最小日志级别
    FLAGS_minloglevel = minLogLevel;

    // 每个日志文件最大大小（MB）
    FLAGS_max_log_size = 10;

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
