//
// Created by Omari on 25-12-17.
//

#ifndef GLOGWRAPPER_H
#define GLOGWRAPPER_H
#include <string>
#include <glog/logging.h>

class GlogWrapper
{
public:
    // 初始化日志系统
    static void Init(const std::string& programName,
                     bool logToConsole = true,
                     int minLogLevel = 0); // 0=INFO, 1=WARNING, 2=ERROR

    // 关闭日志系统
    static void Shutdown();

    static bool IsInitialized();

    explicit GlogWrapper(const std::string& programName,
                     bool logToConsole = true,
                     int minLogLevel = 0);
    ~GlogWrapper();

private:
    static bool s_initialized;
};

#endif //GLOGWRAPPER_H
