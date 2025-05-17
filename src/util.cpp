#include "util.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fstream>
#include <random>
#include <array>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace ImageForensics {

// 初始化静态成员
std::shared_ptr<spdlog::logger> Logger::logger = nullptr;
json Config::configData = json::object();
std::filesystem::path Config::currentConfigPath;

void Logger::init(spdlog::level::level_enum logLevel, const std::optional<std::string>& logFile) {
    if (logger) {
        return; // 已经初始化
    }
    
    try {
        if (logFile) {
            // 创建文件日志
            auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFile.value(), 10 * 1024 * 1024, 3);
            
            // 创建控制台日志
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            
            // 设置日志格式
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
            
            // 创建多输出日志
            std::vector<spdlog::sink_ptr> sinks {consoleSink, fileSink};
            logger = std::make_shared<spdlog::logger>("image_forensics", sinks.begin(), sinks.end());
        } else {
            // 只创建控制台日志
            logger = spdlog::stdout_color_mt("image_forensics");
        }
        
        // 设置日志级别
        logger->set_level(logLevel);
        
        // 设置为默认日志
        spdlog::set_default_logger(logger);
        
        logger->info("Logger initialized");
    } catch (const spdlog::spdlog_ex& ex) {
        fprintf(stderr, "Logger initialization failed: %s\n", ex.what());
        // 创建一个基本的控制台日志作为备用
        logger = spdlog::stdout_color_mt("image_forensics_fallback");
        logger->set_level(spdlog::level::info);
    }
}

std::shared_ptr<spdlog::logger> Logger::get() {
    if (!logger) {
        init(); // 使用默认设置初始化
    }
    return logger;
}

bool Config::load(const std::filesystem::path& configPath) {
    try {
        if (!std::filesystem::exists(configPath)) {
            Logger::get()->warn("Config file not found: {}", configPath.string());
            return false;
        }
        
        std::ifstream file(configPath);
        if (!file.is_open()) {
            Logger::get()->error("Failed to open config file: {}", configPath.string());
            return false;
        }
        
        file >> configData;
        currentConfigPath = configPath;
        
        Logger::get()->info("Loaded config from: {}", configPath.string());
        return true;
    } catch (const std::exception& e) {
        Logger::get()->error("Error loading config: {}", e.what());
        return false;
    }
}

bool Config::save(const std::optional<std::filesystem::path>& configPath) {
    try {
        auto path = configPath.value_or(currentConfigPath);
        
        if (path.empty()) {
            Logger::get()->error("No config path specified");
            return false;
        }
        
        std::ofstream file(path);
        if (!file.is_open()) {
            Logger::get()->error("Failed to open config file for writing: {}", path.string());
            return false;
        }
        
        file << std::setw(4) << configData << std::endl;
        
        Logger::get()->info("Saved config to: {}", path.string());
        return true;
    } catch (const std::exception& e) {
        Logger::get()->error("Error saving config: {}", e.what());
        return false;
    }
}

ImageForensicsException::ImageForensicsException(const std::string& message)
    : std::runtime_error(message) {
    Logger::get()->error("Exception: {}", message);
}

std::string detectMimeType(const std::filesystem::path& filePath) {
    // 简单的文件签名检测
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "application/octet-stream";
    }
    
    // 读取文件头部
    std::array<unsigned char, 12> header;
    file.read(reinterpret_cast<char*>(header.data()), header.size());
    size_t readSize = file.gcount();
    
    // JPEG: FF D8 FF
    if (readSize >= 3 && header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF) {
        return "image/jpeg";
    }
    
    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (readSize >= 8 && header[0] == 0x89 && header[1] == 0x50 && header[2] == 0x4E && header[3] == 0x47 &&
        header[4] == 0x0D && header[5] == 0x0A && header[6] == 0x1A && header[7] == 0x0A) {
        return "image/png";
    }
    
    // GIF: 47 49 46 38
    if (readSize >= 4 && header[0] == 0x47 && header[1] == 0x49 && header[2] == 0x46 && header[3] == 0x38) {
        return "image/gif";
    }
    
    // TIFF: 49 49 2A 00 or 4D 4D 00 2A
    if (readSize >= 4 && 
        ((header[0] == 0x49 && header[1] == 0x49 && header[2] == 0x2A && header[3] == 0x00) ||
         (header[0] == 0x4D && header[1] == 0x4D && header[2] == 0x00 && header[3] == 0x2A))) {
        return "image/tiff";
    }
    
    // BMP: 42 4D
    if (readSize >= 2 && header[0] == 0x42 && header[1] == 0x4D) {
        return "image/bmp";
    }
    
    // 如果无法识别，根据文件扩展名判断
    std::string extension = filePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    } else if (extension == ".png") {
        return "image/png";
    } else if (extension == ".gif") {
        return "image/gif";
    } else if (extension == ".tiff" || extension == ".tif") {
        return "image/tiff";
    } else if (extension == ".bmp") {
        return "image/bmp";
    }
    
    return "application/octet-stream";
}

std::string generateUuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    
    return ss.str();
}

} // namespace ImageForensics 