#include "service.hpp"
#include "metadata.hpp"
#include "storage.hpp"
#include "util.hpp"
#include <future>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace ImageForensics {

// 支持的图像格式
const std::vector<std::string> SUPPORTED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".tiff", ".tif", ".bmp", ".gif"
};

ImageService::ImageService() {
    Logger::get()->info("Initializing image service");
}

json ImageService::processImage(const std::filesystem::path& imagePath) {
    Logger::get()->info("Processing image: {}", imagePath.string());
    
    // 验证图像
    if (!validateImage(imagePath)) {
        Logger::get()->warn("Invalid image file: {}", imagePath.string());
        return {
            {"status", "error"},
            {"message", "Invalid image file"}
        };
    }
    
    // 创建元数据提取器
    MetadataExtractor extractor;
    
    // 提取元数据
    auto metadataOpt = extractor.extractMetadata(imagePath);
    
    if (!metadataOpt) {
        Logger::get()->warn("Failed to extract metadata from: {}", imagePath.string());
        return {
            {"status", "error"},
            {"message", "Failed to extract metadata"}
        };
    }
    
    return {
        {"status", "success"},
        {"metadata", metadataOpt.value()}
    };
}

json ImageService::processBatch(const std::vector<std::filesystem::path>& images) {
    Logger::get()->info("Processing batch of {} images", images.size());
    
    // 存储异步任务
    std::vector<std::future<json>> tasks;
    
    // 为每个图像创建异步任务
    for (const auto& imagePath : images) {
        tasks.push_back(processImageAsync(imagePath));
    }
    
    // 收集结果
    json results = json::array();
    for (auto& task : tasks) {
        results.push_back(task.get());
    }
    
    return {
        {"status", "success"},
        {"results", results}
    };
}

json ImageService::analyzeForensics(const std::filesystem::path& imagePath) {
    Logger::get()->info("Analyzing forensics for image: {}", imagePath.string());
    
    // 验证图像
    if (!validateImage(imagePath)) {
        Logger::get()->warn("Invalid image file: {}", imagePath.string());
        return {
            {"status", "error"},
            {"message", "Invalid image file"}
        };
    }
    
    // 创建元数据提取器
    MetadataExtractor extractor;
    
    // 检测篡改
    auto tamperingOpt = extractor.detectTampering(imagePath);
    
    if (!tamperingOpt) {
        Logger::get()->warn("Failed to analyze forensics for: {}", imagePath.string());
        return {
            {"status", "error"},
            {"message", "Failed to analyze forensics"}
        };
    }
    
    return {
        {"status", "success"},
        {"forensics", tamperingOpt.value()}
    };
}

bool ImageService::validateImage(const std::filesystem::path& imagePath) {
    Logger::get()->info("Validating image: {}", imagePath.string());
    
    // 检查文件是否存在
    if (!std::filesystem::exists(imagePath)) {
        Logger::get()->warn("File does not exist: {}", imagePath.string());
        return false;
    }
    
    // 检查文件大小
    auto fileSize = std::filesystem::file_size(imagePath);
    Logger::get()->info("File size: {} bytes", fileSize);
    
    if (fileSize == 0 || fileSize > 1024 * 1024 * 50) { // 最大50MB
        Logger::get()->warn("Invalid file size: {} bytes", fileSize);
        return false;
    }
    
    // 检查文件扩展名
    auto extension = imagePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    Logger::get()->info("File extension: {}", extension);
    
    if (std::find(SUPPORTED_EXTENSIONS.begin(), SUPPORTED_EXTENSIONS.end(), extension) == SUPPORTED_EXTENSIONS.end()) {
        Logger::get()->warn("Unsupported file extension: {}", extension);
        return false;
    }
    
    // 检查MIME类型
    auto mimeType = detectMimeType(imagePath);
    Logger::get()->info("Detected MIME type: {}", mimeType);
    
    if (mimeType.find("image/") != 0) {
        Logger::get()->warn("Invalid MIME type: {}", mimeType);
        return false;
    }
    
    // 尝试打开图像文件
    try {
        std::ifstream file(imagePath, std::ios::binary);
        if (!file.is_open()) {
            Logger::get()->warn("Failed to open file: {}", imagePath.string());
            return false;
        }
        
        // 读取文件头部
        std::array<unsigned char, 12> header;
        file.read(reinterpret_cast<char*>(header.data()), header.size());
        size_t readSize = file.gcount();
        
        Logger::get()->info("Read {} bytes from file header", readSize);
        
        // 打印文件头部的十六进制值
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < readSize; ++i) {
            ss << std::setw(2) << static_cast<int>(header[i]) << " ";
        }
        Logger::get()->info("File header: {}", ss.str());
    } catch (const std::exception& e) {
        Logger::get()->warn("Exception while reading file: {}", e.what());
        return false;
    }
    
    Logger::get()->info("Image validation successful");
    return true;
}

std::future<json> ImageService::processImageAsync(const std::filesystem::path& imagePath) {
    return std::async(std::launch::async, [this, imagePath]() {
        return this->processImage(imagePath);
    });
}

} // namespace ImageForensics 