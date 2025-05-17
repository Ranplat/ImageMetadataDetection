#pragma once

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <string>
#include <filesystem>
#include <optional>

namespace ImageForensics {

using json = nlohmann::json;

/**
 * @brief 日志管理类
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * @param logLevel 日志级别
     * @param logFile 日志文件路径，如果为空则输出到控制台
     */
    static void init(spdlog::level::level_enum logLevel = spdlog::level::info,
                    const std::optional<std::string>& logFile = std::nullopt);

    /**
     * @brief 获取日志实例
     * @return 日志实例
     */
    static std::shared_ptr<spdlog::logger> get();

private:
    static std::shared_ptr<spdlog::logger> logger;
};

/**
 * @brief 配置管理类
 */
class Config {
public:
    /**
     * @brief 加载配置文件
     * @param configPath 配置文件路径
     * @return 是否成功加载
     */
    static bool load(const std::filesystem::path& configPath);

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    template<typename T>
    static T get(const std::string& key, const T& defaultValue);

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    template<typename T>
    static void set(const std::string& key, const T& value);

    /**
     * @brief 保存配置到文件
     * @param configPath 配置文件路径，如果为空则使用加载时的路径
     * @return 是否成功保存
     */
    static bool save(const std::optional<std::filesystem::path>& configPath = std::nullopt);

private:
    static json configData;
    static std::filesystem::path currentConfigPath;
};

/**
 * @brief 异常类
 */
class ImageForensicsException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param message 异常信息
     */
    explicit ImageForensicsException(const std::string& message);
};

/**
 * @brief 文件类型检测函数
 * @param filePath 文件路径
 * @return 文件MIME类型
 */
std::string detectMimeType(const std::filesystem::path& filePath);

/**
 * @brief 生成UUID
 * @return UUID字符串
 */
std::string generateUuid();

} // namespace ImageForensics

// 模板函数实现
namespace ImageForensics {

template<typename T>
T Config::get(const std::string& key, const T& defaultValue) {
    try {
        if (configData.contains(key)) {
            return configData[key].get<T>();
        }
    } catch (const std::exception& e) {
        Logger::get()->warn("Failed to get config value for key '{}': {}", key, e.what());
    }
    return defaultValue;
}

template<typename T>
void Config::set(const std::string& key, const T& value) {
    configData[key] = value;
}

} // namespace ImageForensics 